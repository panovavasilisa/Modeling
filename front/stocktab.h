#pragma once
#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>

class StockTab : public QWidget {
    Q_OBJECT
public:
    explicit StockTab(QWidget* parent = nullptr);

public slots:
    void refresh();

private:
    QTableView* table;
    QStandardItemModel* model;
};
