#pragma once
#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>

class CouriersTab : public QWidget {
    Q_OBJECT
public:
    explicit CouriersTab(QWidget* parent = nullptr);
public slots:
    void refresh();
private:
    QTableView* table;
    QStandardItemModel* model;
};
