#pragma once
//статистика
class Statistics {
    public:
        double income;  // доходы
        double purchase_expenses; // расходы на закупку
        double write_off_losses; // расходы от списаний

        void update(double add_income, double add_expenses, double add_loss); // обновить статистику
};