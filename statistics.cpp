#include "back/statistics.h"
#include <cmath>
#include <initializer_list>
#include <stdexcept>

void Statistics::update(double add_income, double add_expenses, double add_loss) {
    for (double value : {add_income, add_expenses, add_loss, income,
                        purchase_expenses, write_off_losses})
        if (!std::isfinite(value) || value < 0.0)
            throw std::invalid_argument("Statistics require finite nonnegative amounts");
    const double next_income = income + add_income;
    const double next_expenses = purchase_expenses + add_expenses;
    const double next_losses = write_off_losses + add_loss;
    if (!std::isfinite(next_income) || !std::isfinite(next_expenses) ||
        !std::isfinite(next_losses)) throw std::overflow_error("Statistics overflow");
    income = next_income;
    purchase_expenses = next_expenses;
    write_off_losses = next_losses;
}
