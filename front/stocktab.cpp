#include "stocktab.h"
#include "back/data.h"
#include "back/store.h"
#include "back/medicine.h"

#include <QVBoxLayout>
#include <QHeaderView>

StockTab::StockTab(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    table = new QTableView(this);
    model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({
        "Лекарство", "Дозировка", "Кол-во",
        "Истекает (день)", "Цена за ед."
    });
    table->setModel(model);
    table->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(table);
}

void StockTab::refresh() {
    model->removeRows(0, model->rowCount());
    for (const auto& batch : warehouse.inventory) {
        if (batch.mas_med_id < 0 ||
            batch.mas_med_id >= (int)mas_med.size()) continue;

        const Medicine& m = mas_med[batch.mas_med_id];
        QString name = (m.type_med_id >= 0 &&
                        m.type_med_id < (int)type_med.size())
                           ? QString::fromStdString(type_med[m.type_med_id])
                           : "?";

        QList<QStandardItem*> row;
        row << new QStandardItem(name);
        row << new QStandardItem(QString::number(m.dosage));
        row << new QStandardItem(QString::number(batch.count));
        row << new QStandardItem(QString::number(batch.expiration_day));
        row << new QStandardItem(QString::number(batch.batch_price, 'f', 2));
        model->appendRow(row);
    }
}
