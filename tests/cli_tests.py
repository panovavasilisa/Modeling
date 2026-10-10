"""Проверки собранного приложения Qt с использованием стандартной библиотеки."""
import json
import math
import os
from pathlib import Path
import subprocess
import sys
import tempfile

binary = str(Path(sys.argv[1]).resolve())
checks = 0


def check(condition, message):
    global checks
    checks += 1
    if not condition:
        raise AssertionError(message)


def run(*arguments, success=True):
    environment = dict(os.environ)
    environment.pop("QT_QPA_PLATFORM", None)
    result = subprocess.run([binary, *map(str, arguments)], text=True,
                            capture_output=True, env=environment, timeout=30)
    check(result.returncode == (0 if success else 1),
          f"Unexpected exit code {result.returncode}: {arguments}\n{result.stderr}")
    return result


check("--max-discount" in run("--help").stdout, "help includes configurable discounts")
for arguments in [("--days", "9"), ("--days", "26"), ("--couriers", "2"),
                  ("--couriers", "10"), ("--medicines", "14"), ("--medicines", "36"),
                  ("--markup", "nan"), ("--seed", "-1"), ("--seed", "4294967296"),
                  ("--max-discount", "10"), ("--unknown", "1"), ("--days",)]:
    run(*arguments, success=False)

with tempfile.TemporaryDirectory() as temporary:
    directory = Path(temporary)
    report = directory / "report.json"
    for days, couriers, medicines in [(10, 3, 15), (25, 9, 35)]:
        run("--days", days, "--couriers", couriers, "--medicines", medicines,
            "--quiet", "--report", report, "--seed", "42")
        data = json.loads(report.read_text())
        check(len(data["days"]) == days, "N command-line parameter")
        check(len(data["catalog"]) == medicines, "K command-line parameter")
        check(all(len(day["courier_load"]) == couriers for day in data["days"]), "M command-line parameter")
        check(all(max(day["courier_load"]) <= 15 for day in data["days"]), "maximum courier load")
        totals = data["totals"]
        check(math.isclose(sum(order["price"] for order in data["completed_orders"]), totals["income"], abs_tol=1e-6),
              "JSON income corresponds to real orders")
        check(math.isclose(totals["profit"], totals["income"] - totals["delivered_stock_cost"] - totals["write_off_losses"], abs_tol=1e-6),
              "JSON profit accounts for losses once")

    stock = directory / 'stock with "quotes".csv'
    stock.write_text("medicine_id,count,expiration_day,unit_price\n" +
                     "".join(f"{medicine},10000,365,100\n" for medicine in range(15)))
    arguments = ("--days", "10", "--couriers", "9", "--medicines", "15",
                 "--markup", "20", "--card-discount", "2", "--bulk-discount", "1",
                 "--regular-discount", "3", "--stock", stock, "--quiet", "--report", report, "--seed", "71")
    run(*arguments)
    first = report.read_text()
    data = json.loads(first)
    check(data["parameters"]["initial_stock_file"] == str(stock), "JSON escapes quoted paths")
    check(data["parameters"]["card_discount"] == 2 and data["parameters"]["regular_discount"] == 3,
          "custom discount parameters")
    for purchase in data["completed_orders"]:
        subtotal = sum(item["count"] for item in purchase["supplied"]) * 100 * 1.2
        customer = purchase["customer"]
        discount = 2 if customer["has_card"] else (1 if subtotal > 1000 else 0)
        if customer["is_regular"]:
            discount += 3
        expected = subtotal * (1 - min(discount, 9) / 100)
        check(math.isclose(purchase["price"], expected, abs_tol=1e-8), "actual purchase configurable markup and discounts")
        requested = {item["medicine_id"]: item["count"] for item in purchase["requested"]}
        supplied = {item["medicine_id"]: item["count"] for item in purchase["supplied"]}
        check(requested == supplied, "complete fulfillment from sufficient custom stock")
    run(*arguments)
    check(report.read_text() == first, "whole JSON experiment reproducible")
    stock.write_text("0,-1,50,100\n")
    run("--stock", stock, "--quiet", success=False)

print(f"CLI checks passed: {checks}")
