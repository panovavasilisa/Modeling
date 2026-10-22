#pragma once
#include <QWidget>
#include <QTextEdit>

class ResultsTab : public QWidget {
    Q_OBJECT
public:
    explicit ResultsTab(QWidget* parent = nullptr);
public slots:
    void refresh();
private:
    QTextEdit* text;
};
