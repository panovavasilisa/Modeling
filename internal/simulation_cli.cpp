#include "simulation_cli.h"
#include "model_support.h"
#include "back/data.h"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <ostream>
#include <stdexcept>

namespace model_detail {
namespace {
int integer(const std::string& value) {
    std::size_t end = 0;
    const auto result = std::stoll(value, &end);
    if (end != value.size() || result < 0 || result > std::numeric_limits<int>::max())
        throw std::invalid_argument("Invalid integer: " + value);
    return static_cast<int>(result);
}
double decimal(const std::string& value) {
    std::size_t end = 0;
    const auto result = std::stod(value, &end);
    if (end != value.size() || !std::isfinite(result)) throw std::invalid_argument("Invalid number: " + value);
    return result;
}
std::string quote(const std::string& value) {
    std::string result = "\"";
    const char* hex = "0123456789abcdef";
    for (unsigned char c : value) {
        switch (c) {
        case '\"': result += "\\\""; break;
        case '\\': result += "\\\\"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default:
            if (c < 32) {
                result += "\\u00";
                result += hex[c >> 4];
                result += hex[c & 15];
            } else result += static_cast<char>(c);
        }
    }
    return result + '"';
}
template<class T> void numbers(std::ostream& output, const std::vector<T>& values) {
    output << '[';
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i) output << ',';
        output << values[i];
    }
    output << ']';
}
void items_json(std::ostream& output, const std::vector<std::pair<int, int>>& items) {
    output << '[';
    for (std::size_t i = 0; i < items.size(); ++i) {
        if (i) output << ',';
        output << "{\"medicine_id\":" << items[i].first << ",\"count\":" << items[i].second << '}';
    }
    output << ']';
}
} // анонимное пространство имён

CommandLineOptions parse_command_line(int argc, char* argv[]) {
    CommandLineOptions options;
    for (int i = 1; i < argc; ++i) {
        const std::string option = argv[i];
        if (option == "--help" || option == "-h") { options.help = true; continue; }
        if (option == "--quiet") { options.quiet = true; continue; }
        if (option == "--orders") { options.show_orders = true; continue; }
        if (i + 1 >= argc) throw std::invalid_argument("Missing value for " + option);
        const std::string value = argv[++i];
        auto& p = options.parameters;
        if (option == "--days" || option == "-N") p.days = integer(value);
        else if (option == "--couriers" || option == "-M") p.couriers = integer(value);
        else if (option == "--medicines" || option == "-K") p.medicine_count = integer(value);
        else if (option == "--markup") p.markup_percent = decimal(value);
        else if (option == "--card-discount") p.card_discount = decimal(value);
        else if (option == "--bulk-discount") p.large_purchase_discount = decimal(value);
        else if (option == "--regular-discount") p.regular_discount = decimal(value);
        else if (option == "--max-discount") p.maximum_discount = decimal(value);
        else if (option == "--initial-count") p.initial_count = integer(value);
        else if (option == "--regular-customers") p.regular_customers = integer(value);
        else if (option == "--stock") p.initial_stock_file = value;
        else if (option == "--report") options.report_file = value;
        else if (option == "--seed") {
            std::size_t end = 0;
            const auto seed = std::stoull(value, &end);
            if (value.empty() || value[0] == '-' || end != value.size() || seed > std::numeric_limits<std::uint32_t>::max())
                throw std::invalid_argument("Seed must be between 0 and 4294967295");
            p.seed = static_cast<std::uint32_t>(seed);
        } else throw std::invalid_argument("Unknown option: " + option);
    }
    if (!options.help) options.parameters.validate();
    return options;
}

void print_help(std::ostream& output) {
    output << "PharmDelivery: модель доставки лекарств, один шаг — один день\n"
           << "  --days N (-N)                 10–25, по умолчанию 15\n"
           << "  --couriers M (-M)             3–9, по умолчанию 5\n"
           << "  --medicines K (-K)            15–35, по умолчанию 25\n"
           << "  --markup P                   розничная наценка %, по умолчанию 25\n"
           << "  --card-discount P            скидка по карте %, по умолчанию 5\n"
           << "  --bulk-discount P            без карты при покупке >1000 руб., по умолчанию 3\n"
           << "  --regular-discount P         дополнительная скидка постоянному клиенту %, 5\n"
           << "  --max-discount P             предел общей скидки, 0–9%, по умолчанию 9\n"
           << "  --initial-count C            начальные упаковки каждого лекарства, 80\n"
           << "  --stock FILE                 начальные партии вместо генерации\n"
           << "                               medicine_id,count,expiration_day[,unit_price]\n"
           << "  --regular-customers C        число постоянных покупателей, 10\n"
           << "  --seed S                     воспроизводимость эксперимента, 5489\n"
           << "  --report FILE                сохранить JSON с остатками и доставками\n"
           << "  --orders                     печатать сведения каждой доставки\n"
           << "  --quiet                      печатать только итог\n"
           << "  --help                       показать справку\n";
}

void print_catalog(std::ostream& output) {
    output << "Каталог (индексы medicine_id для --stock):\n";
    for (std::size_t i = 0; i < mas_med.size(); ++i) {
        const auto& medicine = mas_med[i];
        output << '#' << i << ": " << type_med.at(medicine.type_med_id) << ", дозировка "
               << medicine.dosage << " мг, группа " << group_med.at(medicine.group_med_id)
               << ", опт " << medicine.wholesale_price << ", срок " << medicine.shelf_life
               << " дней, минимум " << medicine.min_stock << '\n';
    }
}

void print_daily_report(std::ostream& output, const SimulationResult& result, bool show_orders) {
    const auto& day = result.days.back();
    output << "\nДень " << day.day << ": заказов " << day.received_orders
           << " (плановых " << day.planned_orders << "), доставлено " << day.delivered_orders
           << ", подготовлено на завтра " << day.prepared_orders
           << ", без товара " << day.empty_orders << ", очередь " << day.pending_orders
           << ", поставок " << day.arrived_supplies << ", новых заявок " << day.created_supplies << '\n';
    output << "Загрузка курьеров: ";
    for (auto count : day.courier_load) output << count << ' ';
    output << (day.courier_limits_satisfied ? "(7–15 выполнено)\n" : "(заказов недостаточно для минимума 7 у каждого)\n");
    output << "Остатки: ";
    for (std::size_t i = 0; i < day.available_packages.size(); ++i) output << '#' << i << '=' << day.available_packages[i] << ' ';
    output << '\n';
    if (day.late_deliveries) output << "Доставлено позже следующего дня из-за очереди: " << day.late_deliveries << '\n';
    if (show_orders) for (const auto& purchase : result.completed_orders) {
        if (purchase.delivered_day != day.day) continue;
        output << "Заказ " << purchase.order_id << ", курьер " << purchase.courier_index + 1 << ", "
               << purchase.customer.fio << ", " << purchase.customer.phone << ", " << purchase.customer.address
               << ", выдано: ";
        for (const auto& item : purchase.supplied) output << '#' << item.first << 'x' << item.second << ' ';
        output << ", стоимость " << std::fixed << std::setprecision(2) << purchase.price << " руб.\n";
    }
}

void write_json_report(const std::string& path) {
    std::ofstream output(path);
    if (!output) throw std::runtime_error("Cannot create JSON report: " + path);
    output << std::setprecision(17);
    const auto& p = simulation_parameters();
    const auto& result = simulation_result();
    output << "{\n\"parameters\":{\"N\":" << p.days << ",\"M\":" << p.couriers << ",\"K\":" << p.medicine_count
           << ",\"seed\":" << p.seed << ",\"markup_percent\":" << p.markup_percent
           << ",\"card_discount\":" << p.card_discount << ",\"bulk_discount\":" << p.large_purchase_discount
           << ",\"regular_discount\":" << p.regular_discount << ",\"maximum_discount\":" << p.maximum_discount
           << ",\"initial_count\":" << p.initial_count << ",\"regular_customers\":" << p.regular_customers
           << ",\"initial_stock_file\":" << quote(p.initial_stock_file) << "},\n";
    output << "\"totals\":{\"income\":" << result.totals.income
           << ",\"purchase_expenses\":" << result.totals.purchase_expenses
           << ",\"write_off_losses\":" << result.totals.write_off_losses
           << ",\"profit\":" << result.profit << ",\"available_stock_cost\":" << result.available_stock_cost
           << ",\"reserved_stock_cost\":" << result.reserved_stock_cost << ",\"delivered_stock_cost\":" << result.delivered_stock_cost
           << ",\"initial_packages\":" << result.initial_packages << ",\"supplied_packages\":" << result.supplied_packages
           << ",\"written_off_packages\":" << result.written_off_packages
           << ",\"pending_deliveries\":" << result.pending_deliveries << ",\"pending_orders\":" << result.pending_orders << "},\n";
    output << "\"catalog\":[";
    for (std::size_t i = 0; i < mas_med.size(); ++i) {
        if (i) output << ',';
        const auto& med = mas_med[i];
        output << "{\"medicine_id\":" << i << ",\"dosage\":" << med.dosage
               << ",\"type\":" << quote(type_med.at(med.type_med_id)) << ",\"group\":" << quote(group_med.at(med.group_med_id))
               << ",\"wholesale_price\":" << med.wholesale_price << ",\"shelf_life\":" << med.shelf_life << ",\"min_stock\":" << med.min_stock << '}';
    }
    output << "],\n\"days\":[";
    for (std::size_t i = 0; i < result.days.size(); ++i) {
        if (i) output << ',';
        const auto& day = result.days[i];
        output << "{\"day\":" << day.day << ",\"received_orders\":" << day.received_orders
               << ",\"planned_orders\":" << day.planned_orders << ",\"prepared_orders\":" << day.prepared_orders
               << ",\"delivered_orders\":" << day.delivered_orders << ",\"empty_orders\":" << day.empty_orders
               << ",\"late_deliveries\":" << day.late_deliveries << ",\"pending_orders\":" << day.pending_orders
               << ",\"arrived_supplies\":" << day.arrived_supplies << ",\"created_supplies\":" << day.created_supplies
               << ",\"income\":" << day.totals.income << ",\"purchase_expenses\":" << day.totals.purchase_expenses
               << ",\"write_off_losses\":" << day.totals.write_off_losses << ",\"courier_limits_satisfied\":"
               << (day.courier_limits_satisfied ? "true" : "false") << ",\"courier_load\":";
        numbers(output, day.courier_load);
        output << ",\"available_packages\":";
        numbers(output, day.available_packages);
        output << '}';
    }
    output << "],\n\"completed_orders\":[";
    for (std::size_t i = 0; i < result.completed_orders.size(); ++i) {
        if (i) output << ',';
        const auto& purchase = result.completed_orders[i];
        output << "{\"order_id\":" << purchase.order_id << ",\"ordered_day\":" << purchase.ordered_day
               << ",\"prepared_day\":" << purchase.prepared_day << ",\"delivered_day\":" << purchase.delivered_day
               << ",\"courier_index\":" << purchase.courier_index << ",\"price\":" << purchase.price
               << ",\"acquisition_cost\":" << purchase.acquisition_cost << ",\"customer\":{\"fio\":" << quote(purchase.customer.fio)
               << ",\"phone\":" << quote(purchase.customer.phone) << ",\"address\":" << quote(purchase.customer.address)
               << ",\"has_card\":" << (purchase.customer.has_card ? "true" : "false")
               << ",\"is_regular\":" << (purchase.customer.is_regular ? "true" : "false")
               << ",\"card_number\":" << purchase.customer.card_number << "},\"requested\":";
        items_json(output, purchase.requested);
        output << ",\"supplied\":";
        items_json(output, purchase.supplied);
        output << '}';
    }
    output << "],\n\"inventory\":[";
    for (std::size_t i = 0; i < warehouse.inventory.size(); ++i) {
        if (i) output << ',';
        const auto& batch = warehouse.inventory[i];
        output << "{\"medicine_id\":" << batch.mas_med_id << ",\"count\":" << batch.count
               << ",\"expiration_day\":" << batch.expiration_day << ",\"unit_price\":" << batch.batch_price << '}';
    }
    output << "],\n\"pending_supplies\":[";
    for (std::size_t i = 0; i < result.pending_supplies.size(); ++i) {
        if (i) output << ',';
        const auto& supply = result.pending_supplies[i];
        output << "{\"medicine_id\":" << supply.mas_med_id << ",\"count\":" << supply.count
               << ",\"delivery_timer\":" << supply.delivery_timer << '}';
    }
    output << "]\n}\n";
    output.close();
    if (!output) throw std::runtime_error("Failed writing JSON report: " + path);
}

} // пространство имён model_detail
