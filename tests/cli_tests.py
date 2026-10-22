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


help_text = run("--help").stdout
for option in ("--max-discount", "--orders-per-day", "--initial-capital", "--restock-threshold",
               "--markdown-days", "--discounted-probability"):
    check(option in help_text, f"help includes {option}")
for arguments in [("--days", "9"), ("--days", "26"), ("--couriers", "2"),
                  ("--couriers", "10"), ("--medicines", "14"), ("--medicines", "36"),
                  ("--markup", "nan"), ("--seed", "-1"), ("--seed", "4294967296"),
                  ("--max-discount", "10"), ("--unknown", "1"), ("--days",),
                  ("--orders-per-day", "-1"), ("--orders-per-day", "100001"),
                  ("--orders-per-day", "1.5"), ("--orders-per-day",),
                  ("--initial-capital", "-1"), ("--initial-capital", "nan"),
                  ("--initial-capital", "inf"), ("--restock-threshold", "-1"),
                  ("--restock-threshold", "1000001"), ("--markdown-days", "-1"),
                  ("--markdown-days", "1000001"), ("--discounted-probability", "-0.1"),
                  ("--discounted-probability", "1.1"), ("--discounted-probability", "nan"),
                  ("--discounted-probability", "inf")]:
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
        check(all(day["received_orders"] <= data["parameters"]["orders_per_day"] for day in data["days"]),
              "JSON daily arrivals respect configured limit")
        totals = data["totals"]
        check(math.isclose(sum(order["price"] for order in data["completed_orders"]), totals["income"], abs_tol=1e-6),
              "JSON income corresponds to real orders")
        check(math.isclose(totals["profit"], totals["income"] - totals["delivered_stock_cost"] - totals["write_off_losses"], abs_tol=1e-6),
              "JSON profit accounts for losses once")
        for balance in [totals, *data["days"]]:
            check(math.isclose(balance["cash_balance"], data["parameters"]["initial_capital"] +
                               balance["income"] - balance["purchase_expenses"], abs_tol=1e-6),
                  "JSON cash includes capital and actual cash flows")

    for limit in (0, 1, 10):
        run("--days", 10, "--orders-per-day", limit, "--markup", 0,
            "--regular-customers", 100, "--initial-count", 0, "--restock-threshold", 0,
            "--initial-capital", 0, "--discounted-probability", 0, "--markdown-days", 0,
            "--quiet", "--report", report)
        data = json.loads(report.read_text())
        check(data["parameters"]["orders_per_day"] == limit, "CLI order limit reaches model")
        check(all(day["received_orders"] == limit for day in data["days"]),
              "zero markup fills exact daily limit including regular customers")
        check(data["totals"]["cash_balance"] == 0 and data["totals"]["supplied_packages"] == 0,
              "zero threshold and empty stock preserve zero capital")
        check(all(medicine["min_stock"] == 0 for medicine in data["catalog"]),
              "CLI threshold reaches entire catalog")

    stock = directory / 'stock with "quotes".csv'
    stock.write_text("medicine_id,count,expiration_day,unit_price\n" +
                     "".join(f"{medicine},10000,365,100\n" for medicine in range(15)))
    arguments = ("--days", "10", "--couriers", "9", "--medicines", "15",
                 "--markup", "20", "--card-discount", "2", "--bulk-discount", "1",
                 "--orders-per-day", "10", "--initial-capital", "125.5", "--restock-threshold", "11",
                 "--markdown-days", "5", "--discounted-probability", "0.25",
                 "--regular-discount", "3", "--stock", stock, "--quiet", "--report", report, "--seed", "71")
    run(*arguments)
    first = report.read_text()
    data = json.loads(first)
    check(data["parameters"]["initial_stock_file"] == str(stock), "JSON escapes quoted paths")
    check(data["parameters"]["card_discount"] == 2 and data["parameters"]["regular_discount"] == 3,
          "custom discount parameters")
    for name, expected in {"orders_per_day": 10, "initial_capital": 125.5, "restock_threshold": 11,
                           "markdown_days": 5, "discounted_probability": 0.25}.items():
        check(data["parameters"][name] == expected, f"JSON custom parameter {name}")
    check(all(day["received_orders"] <= 10 for day in data["days"]), "custom limit bounds actual arrivals")
    check(all(medicine["min_stock"] == 11 for medicine in data["catalog"]), "custom catalog threshold")
    check(data["totals"]["cash_balance"] < 0, "initial capital permits negative recorded balance")
    check(bool(data["completed_orders"]), "custom scenario exercises actual deliveries")
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
