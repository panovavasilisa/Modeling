#pragma once
#include <QMainWindow>
#include <QTimer>

#include "back/simulation.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_generateBtn_clicked();
    void on_startBtn_clicked();
    void on_pauseBtn_clicked();
    void on_stepBtn_clicked();
    void on_resetBtn_clicked();
    void onSpeedChanged(int value);
    void tick();

private:
    Ui::MainWindow* ui;
    QTimer* timer;
    int day;
    int totalDays;

    SimulationParameters buildParameters() const;
    void doStep();
    void refreshAllTabs();
};
