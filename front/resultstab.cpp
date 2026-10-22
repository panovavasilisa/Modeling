#include "resultstab.h"
#include "back/simulation.h"
#include <QVBoxLayout>

ResultsTab::ResultsTab(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    text = new QTextEdit(this);
    text->setReadOnly(true);
    text->setFontFamily("Consolas");
    layout->addWidget(text);
}

void ResultsTab::refresh() {
    const auto& r = simulation_result();
    QString s;
    s += "=== ИТОГИ СИМУЛЯЦИИ ===\n\n";
    s += QString("Доход:              %1 руб.\n").arg(r.totals.income, 0, 'f', 2);
    s += QString("Расходы на закупку: %1 руб.\n").arg(r.totals.purchase_expenses, 0, 'f', 2);
    s += QString("Потери от списания: %1 руб.\n").arg(r.totals.write_off_losses, 0, 'f', 2);
    s += QString("Прибыль:            %1 руб.\n").arg(r.profit, 0, 'f', 2);
    s += "\n";
    s += QString("Стоимость остатков: %1 руб.\n").arg(r.available_stock_cost, 0, 'f', 2);
    s += QString("Партий изначально:  %1\n").arg(r.initial_packages);
    s += QString("Поставлено (шт):    %1\n").arg(r.supplied_packages);
    s += QString("Списано (шт):       %1\n").arg(r.written_off_packages);
    s += "\n";
    s += QString("Доставленных заказов: %1\n").arg(r.completed_orders.size());
    s += QString("Ожидают доставки:     %1\n").arg(r.pending_deliveries);
    s += QString("Ожидают обработки:    %1\n").arg(r.pending_orders);
    s += QString("Ожидают поставки:     %1\n").arg(r.pending_supplies.size());
    text->setPlainText(s);
}
