#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "back/data.h"
#include "internal/model_support.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), day(0), totalDays(15)
{
    ui->setupUi(this);

    // Таймер
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::tick);

    // Скорость
    ui->horizontalSlider->setMinimum(1);
    ui->horizontalSlider->setMaximum(10);
    ui->horizontalSlider->setValue(5);
    connect(ui->horizontalSlider, &QSlider::valueChanged,
            this, &MainWindow::onSpeedChanged);
    onSpeedChanged(5);

    // Диапазоны по ТЗ
    ui->daysSpin->setRange(10, 25);        ui->daysSpin->setValue(15);
    ui->couriersSpin->setRange(3, 9);      ui->couriersSpin->setValue(5);
    ui->medicinesSpin->setRange(15, 35);   ui->medicinesSpin->setValue(25);
    ui->markupSpin->setRange(0.0, 100.0);  ui->markupSpin->setValue(25.0);
    ui->cardDiscountSpin->setRange(0.0, 9.0);     ui->cardDiscountSpin->setValue(5.0);
    ui->regularDiscountSpin->setRange(0.0, 9.0);  ui->regularDiscountSpin->setValue(5.0);
    ui->bigOrderDiscountSpin->setRange(0.0, 9.0); ui->bigOrderDiscountSpin->setValue(3.0);
    ui->maxDiscountSpin->setRange(0.0, 9.0);      ui->maxDiscountSpin->setValue(9.0);
    ui->thresholdSpin->setValue(10);
    ui->expiryDaysSpin->setValue(30);
    ui->ordersPerDaySpin->setValue(20);
    ui->discountedProbSpin->setRange(0, 100);
    ui->discountedProbSpin->setValue(30);

    refreshAllTabs();
}

MainWindow::~MainWindow() { delete ui; }

SimulationParameters MainWindow::buildParameters() const {
    SimulationParameters p;
    p.days               = ui->daysSpin->value();
    p.couriers           = ui->couriersSpin->value();
    p.medicine_count     = ui->medicinesSpin->value();
    p.markup_percent     = ui->markupSpin->value();
    p.card_discount      = ui->cardDiscountSpin->value();
    p.large_purchase_discount = ui->bigOrderDiscountSpin->value();
    p.regular_discount   = ui->regularDiscountSpin->value();
    p.maximum_discount   = ui->maxDiscountSpin->value();
    p.regular_customers  = 10;
    p.seed               = 5489;
    return p;
}

void MainWindow::on_generateBtn_clicked() {
    timer->stop();
    day = 0;
    ui->lineEdit->setText("0");

    const auto params = buildParameters();
    totalDays = params.days;

    ui->logTab->clear();
    ui->logTab->append("=== Инициализация симуляции ===");
    ui->logTab->append(QString("Дней: %1, Курьеров: %2, Лекарств: %3")
                           .arg(params.days).arg(params.couriers).arg(params.medicine_count));
    ui->logTab->append(QString("Наценка: %1%").arg(params.markup_percent));

    try {
        configure_simulation(params);
        init_simulation();
    } catch (const std::exception& e) {
        ui->logTab->append(QString("Ошибка: %1").arg(e.what()));
        return;
    }

    ui->logTab->append(QString("Партий на складе: %1").arg(warehouse.inventory.size()));
    ui->logTab->append(QString("Покупателей: %1").arg(customers.size()));
    ui->logTab->append("---");

    refreshAllTabs();
    ui->tabs->setCurrentWidget(ui->stockTab);
}

void MainWindow::on_startBtn_clicked() {
    if (day == 0) {
        // Первый запуск — инициализация
        on_generateBtn_clicked();
    }
    timer->start();
}

void MainWindow::on_pauseBtn_clicked() { timer->stop(); }
void MainWindow::on_stepBtn_clicked()  { doStep(); }

void MainWindow::on_resetBtn_clicked() {
    timer->stop();
    day = 0;
    ui->lineEdit->setText("0");
    ui->logTab->clear();
    ui->logTab->append("=== Сброс ===");
    refreshAllTabs();
}

void MainWindow::onSpeedChanged(int value) {
    int interval = 2000 - (value - 1) * 210;
    if (interval < 100) interval = 100;
    timer->setInterval(interval);
}

void MainWindow::tick() {
    doStep();
    if (day >= totalDays) timer->stop();
}

void MainWindow::doStep() {
    if (day >= totalDays) return;
    day++;
    ui->lineEdit->setText(QString::number(day));

    try {
        manage_warehouse(day);
        generate_daily_events(day);
        process_daily_order(day);
        cleanup_random_customers();
    } catch (const std::exception& e) {
        ui->logTab->append(QString("[День %1] Ошибка: %2").arg(day).arg(e.what()));
        return;
    }

    const auto& r = simulation_result();
    if (!r.days.empty()) {
        const auto& d = r.days.back();
        ui->logTab->append(QString("[День %1] заказов: %2, доставлено: %3, "
                                   "заявок создано: %1, прибыло: %4")
                               .arg(day)
                               .arg(d.received_orders)
                               .arg(d.delivered_orders)
                               .arg(d.created_supplies)
                               .arg(d.arrived_supplies));
    }

    refreshAllTabs();
}

void MainWindow::refreshAllTabs() {
    ui->stockTab->refresh();
    // следующие вкладки подключим отдельно:
    // ui->ordersTab->refresh();
    // ui->couriersTab->refresh();
    // ui->requestsTab->refresh();
}
