#pragma once
#include <vector>

//заказ
class Order{
    public:
        int customer_idx;  // индекс массива клиентов
        std::vector <std::pair<int, int>> items; // список заказа (индекс из mas_med и кол-во)
        double final_price; // цена с учетом скидок
};