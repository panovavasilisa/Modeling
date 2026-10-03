#pragma once
#include <string>
#include <vector>

// покупатель
class Customer{
    public:
        std::string fio;
        std::string phone;
        std::string address;
        bool has_card;
        bool is_regular;
        int card_number;

        std::vector<std::pair<int, int>> regular_purchases;  // постоянные покупки если !is_regular то nullptr
        int periodicity;  // переодичность постоянной покупки
    }; 
