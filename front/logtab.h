#pragma once
#include <QWidget>
#include <QTextEdit>

class LogTab : public QWidget {
    Q_OBJECT
public:
    explicit LogTab(QWidget* parent = nullptr);

public slots:
    void append(const QString& line);   // добавить строку
    void clear();                       // очистить журнал

private:
    QTextEdit* text;
};
