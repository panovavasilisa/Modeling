#pragma once
#include <vector>
#include <string>
#include "data.h"

class DataGenerator {
    public:
    
        void generate_catalog(int K); // генерация каталога лекарств
        void generate_warehouse(int current_day); // генерация начальных партий на складе
        void generate_customers(int cnt); // генерация постоянных покупателей
        void generate_random_customers(); // генерация случайного клиента
        bool should_generate_order(double markup_percent); // проверка появления заказа
        std::vector <std::pair<int, int>> generate_order_items(); //генерация позиций дял заказа
        int generate_delivery_timer(); // генерция таймера для доставки для заявки поставщику
};