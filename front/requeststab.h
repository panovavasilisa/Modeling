#pragma once
#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>

class RequestsTab : public QWidget {
    Q_OBJECT
public:
    explicit RequestsTab(QWidget* parent = nullptr);
public slots:
    void refresh();
private:
    QTableView* table;
    QStandardItemModel* model;
};
