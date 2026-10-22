#include "courierstab.h"
#include "back/simulation.h"
#include <QVBoxLayout>
#include <QHeaderView>

CouriersTab::CouriersTab(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    table = new QTableView(this);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({
        "Курьер", "Заказов за день", "Всего за симуляцию", "В пределах 7–15"
    });
    table->setModel(model);
    table->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(table);
}

void CouriersTab::refresh() {
    model->removeRows(0, model->rowCount());
    const auto& r = simulation_result();
    if (r.days.empty()) return;

    const auto& last = r.days.back();
    const int M = static_cast<int>(last.courier_load.size());

    std::vector<int> total(M, 0);
    for (const auto& d : r.days)
        for (int i = 0; i < M && i < (int)d.courier_load.size(); ++i)
            total[i] += d.courier_load[i];

    for (int i = 0; i < M; ++i) {
        const int load = last.courier_load[i];
        const bool ok = (load >= 7 && load <= 15);
        QList<QStandardItem*> row;
        row << new QStandardItem(QString("Курьер %1").arg(i + 1));
        row << new QStandardItem(QString::number(load));
        row << new QStandardItem(QString::number(total[i]));
        row << new QStandardItem(ok ? "Норма" : load < 7 ? "Недогрузка" : "Перегрузка");
        for (auto* item : row) item->setForeground(QColor(ok ? "#087f70" : "#a96c20"));
        model->appendRow(row);
    }
    table->resizeColumnsToContents();
}
