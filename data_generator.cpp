#include "back/data_generator.h"
#include "internal/model_support.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

void DataGenerator::generate_catalog(int count) {
    if (count < 15 || count > 35) throw std::invalid_argument("K must be between 15 and 35");
    type_med = {"таблетки", "суспензия", "спрей", "мазь", "капсулы", "раствор"};
    group_med = {"сердечно-сосудистые", "антибиотики", "эндокринные", "обезболивающие", "противоаллергические"};
    mas_med.clear();
    ::K = count;
    for (int i = 0; i < count; ++i) {
        mas_med.push_back(Medicine{25 * (1 + i / 6), i % 6, i % 5,
            static_cast<double>(model_detail::random_int(50, 600)),
            model_detail::random_int(45, 180), model_detail::random_int(10, 25)});
    }
}

void DataGenerator::generate_warehouse(int current_day) {
    if (current_day < 0 || model_detail::context().initial_count < 0)
        throw std::invalid_argument("Invalid initial warehouse settings");
    warehouse.inventory.clear();
    model_detail::context().warehouses.erase(&warehouse);
    for (std::size_t i = 0; i < mas_med.size(); ++i) {
        const auto& medicine = mas_med[i];
        const int count = model_detail::context().initial_count;
        if (count == 0) continue;
        warehouse.add_batch(StoreBatch{static_cast<int>(i), count / 2,
            current_day + model_detail::random_int(1, medicine.shelf_life), medicine.wholesale_price});
        warehouse.add_batch(StoreBatch{static_cast<int>(i), count - count / 2,
            current_day + medicine.shelf_life, medicine.wholesale_price});
    }
}

void DataGenerator::generate_customers(int cnt) {
    if (cnt < 0 || cnt > 100000) throw std::invalid_argument("Invalid customer count");
    customers.clear();
    for (int i = 0; i < cnt; ++i) {
        Customer customer{};
        customer.fio = "Постоянный покупатель " + std::to_string(i + 1);
        customer.phone = "+7900" + std::to_string(1000000 + i);
        customer.address = "ул. Аптечная, " + std::to_string(i + 1);
        customer.has_card = model_detail::random_int(0, 1) != 0;
        customer.is_regular = true;
        customer.card_number = customer.has_card ? 10000 + i : 0;
        customer.periodicity = model_detail::random_int(3, 7);
        customer.next_purchase_day = model_detail::random_int(1, customer.periodicity);
        customer.regular_purchases = generate_order_items();
        customers.push_back(std::move(customer));
    }
}

void DataGenerator::generate_random_customers() {
    static const char* surnames[] = {"Иванов", "Петров", "Смирнова", "Кузнецова", "Соколов", "Попова"};
    Customer customer{};
    customer.fio = std::string(surnames[model_detail::random_int(0, 5)]) + " " +
        std::to_string(model_detail::random_int(1000, 9999));
    customer.phone = "+7900" + std::to_string(model_detail::random_int(1000000, 9999999));
    customer.address = "ул. Доставочная, " + std::to_string(model_detail::random_int(1, 150));
    customer.has_card = model_detail::random_int(0, 1) != 0;
    customer.is_regular = false;
    customer.card_number = customer.has_card ? model_detail::random_int(20000, 99999) : 0;
    customers.push_back(std::move(customer));
}

bool DataGenerator::should_generate_order(double markup_percent) {
    if (!std::isfinite(markup_percent) || markup_percent < 0.0)
        throw std::invalid_argument("Invalid retail markup");
    // Independent arrival opportunities approximate a binomial daily stream.
    const double probability = 0.75 * std::exp(-markup_percent / 60.0);
    return std::bernoulli_distribution(probability)(model_detail::context().random);
}

std::vector<std::pair<int, int>> DataGenerator::generate_order_items() {
    if (mas_med.empty()) return {};
    std::vector<double> weights(mas_med.size(), 1.0);
    for (std::size_t i = 0; i < weights.size(); ++i)
        if (model_detail::context().is_discounted(warehouse, static_cast<int>(i))) weights[i] = 3.0;
    const int positions = model_detail::random_int(1, std::min(4, static_cast<int>(mas_med.size())));
    std::vector<std::pair<int, int>> result;
    for (int i = 0; i < positions; ++i) {
        const auto selected = std::discrete_distribution<std::size_t>(weights.begin(), weights.end())
            (model_detail::context().random);
        result.emplace_back(static_cast<int>(selected), model_detail::random_int(1, 4));
        weights[selected] = 0.0;
    }
    return result;
}

int DataGenerator::generate_delivery_timer() {
    return model_detail::random_int(1, 3);
}
