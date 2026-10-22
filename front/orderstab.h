#pragma once
#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>

class OrdersTab : public QWidget {
    Q_OBJECT
public:
    explicit OrdersTab(QWidget* parent = nullptr);
public slots:
    void refresh();
private:
    QTableView* table;
    QStandardItemModel* model;
};
