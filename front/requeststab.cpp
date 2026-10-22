#include "requeststab.h"
#include "back/simulation.h"
#include "back/data.h"
#include <QVBoxLayout>
#include <QHeaderView>

RequestsTab::RequestsTab(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    table = new QTableView(this);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({
        "ID", "Лекарство", "Кол-во", "Ожидается через (дней)"
    });
    table->setModel(model);
    table->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(table);
}

void RequestsTab::refresh() {
    model->removeRows(0, model->rowCount());
    const auto& r = simulation_result();
    for (const auto& req : r.pending_supplies) {
        QString name = "?";
        if (req.mas_med_id >= 0 && req.mas_med_id < (int)type_med.size()) {
            const auto& m = mas_med[req.mas_med_id];
            if (m.type_med_id >= 0 && m.type_med_id < (int)type_med.size())
                name = QString::fromStdString(type_med[m.type_med_id])
                       + " (" + QString::number(m.dosage) + " мг)";
        }
        QList<QStandardItem*> row;
        row << new QStandardItem(QString::number(req.mas_med_id));
        row << new QStandardItem(name);
        row << new QStandardItem(QString::number(req.count));
        row << new QStandardItem(QString::number(req.delivery_timer));
        model->appendRow(row);
    }
    table->resizeColumnsToContents();
}
