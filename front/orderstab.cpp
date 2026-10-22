#include "orderstab.h"
#include "back/simulation.h"
#include "back/data.h"
#include <QVBoxLayout>
#include <QHeaderView>

OrdersTab::OrdersTab(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    table = new QTableView(this);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({
        "№", "Заказ", "Доставка", "Курьер", "ФИО", "Телефон", "Адрес", "Карта",
        "Запрошено", "Выдано", "Цена"
    });
    table->setModel(model);
    table->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(table);
}

void OrdersTab::refresh() {
    model->removeRows(0, model->rowCount());
    const auto& r = simulation_result();
    for (const auto& o : r.completed_orders) {
        QList<QStandardItem*> row;
        row << new QStandardItem(QString::number(o.order_id));
        row << new QStandardItem(QString::number(o.ordered_day));
        row << new QStandardItem(o.delivered_day > 0
                                     ? QString::number(o.delivered_day)
                                     : QString("—"));
        row << new QStandardItem(o.courier_index >= 0
                                     ? QString("Курьер %1").arg(o.courier_index + 1)
                                     : QString("—"));
        row << new QStandardItem(QString::fromStdString(o.customer.fio));
        row << new QStandardItem(QString::fromStdString(o.customer.phone));
        row << new QStandardItem(QString::fromStdString(o.customer.address));
        row << new QStandardItem(o.customer.has_card ? QString::number(o.customer.card_number) : "—");

        QStringList req, sup;
        for (auto& p : o.requested) req << QString("%1×%2").arg(p.first).arg(p.second);
        for (auto& p : o.supplied)  sup << QString("%1×%2").arg(p.first).arg(p.second);
        row << new QStandardItem(req.join(", "));
        row << new QStandardItem(sup.isEmpty() ? "не выдано" : sup.join(", "));

        row << new QStandardItem(QString::number(o.price, 'f', 2));
        model->appendRow(row);
    }
    table->resizeColumnsToContents();
}
