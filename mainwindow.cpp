#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "back/data.h"
#include "back/data_generator.h"
#include "internal/model_support.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), day(0), totalDays(15)
{
    ui->setupUi(this);

    // Таймер симуляции
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
    ui->daysSpin->setRange(10, 25);
    ui->daysSpin->setValue(15);
    ui->couriersSpin->setRange(3, 9);
    ui->couriersSpin->setValue(4);
    ui->medicinesSpin->setRange(15, 35);
    ui->medicinesSpin->setValue(20);
    ui->markupSpin->setRange(0.0, 100.0);
    ui->markupSpin->setValue(25.0);
    ui->cardDiscountSpin->setRange(0.0, 9.0);
    ui->cardDiscountSpin->setValue(5.0);
    ui->regularDiscountSpin->setRange(0.0, 9.0);
    ui->regularDiscountSpin->setValue(5.0);
    ui->bigOrderDiscountSpin->setRange(0.0, 9.0);
    ui->bigOrderDiscountSpin->setValue(3.0);
    ui->maxDiscountSpin->setRange(0.0, 9.0);
    ui->maxDiscountSpin->setValue(9.0);

    refreshAllTabs();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_generateBtn_clicked() {
    // 1. Сброс глобального состояния модели
    model_detail::context().reset(42);
    warehouse.inventory.clear();

    // 2. Считываем параметры из UI
    K = ui->medicinesSpin->value();
    N = ui->daysSpin->value();
    M = ui->couriersSpin->value();
    global_marcup = ui->markupSpin->value();
    totalDays = N;

    // 3. Скидки — в контекст
    model_detail::context().discounts.card_percent          = ui->cardDiscountSpin->value();
    model_detail::context().discounts.regular_percent       = ui->regularDiscountSpin->value();
    model_detail::context().discounts.large_purchase_percent= ui->bigOrderDiscountSpin->value();
    model_detail::context().discounts.maximum_percent       = ui->maxDiscountSpin->value();

    // 4. Генерация
    DataGenerator gen;
    gen.generate_catalog(K);
    gen.generate_warehouse(0);
    gen.generate_customers(10);   // 10 постоянных покупателей

    // 5. Обновить UI
    refreshAllTabs();

    ui->logTab->clear();
    ui->logTab->append(QString("=== Симуляция инициализирована ==="));
    ui->logTab->append(QString("Дней (N): %1, Курьеров (M): %2, Лекарств (K): %3")
                           .arg(N).arg(M).arg(K));
    ui->logTab->append(QString("Наценка: %1%, Скидка по карте: %2%")
                           .arg(global_marcup).arg(ui->cardDiscountSpin->value()));
    ui->logTab->append(QString("Партий на складе: %1").arg(warehouse.inventory.size()));
    ui->logTab->append(QString("Покупателей: %1").arg(customers.size()));
    ui->logTab->append("---");

    // Переключиться на вкладку «Склад», чтобы сразу увидеть результат
    ui->tabs->setCurrentWidget(ui->stockTab);
}

void MainWindow::on_startBtn_clicked() { timer->start(); }
void MainWindow::on_pauseBtn_clicked() { timer->stop(); }
void MainWindow::on_stepBtn_clicked()  { doStep(); }

void MainWindow::on_resetBtn_clicked() {
    timer->stop();
    day = 0;
    ui->lineEdit->setText("0");
    refreshAllTabs();
    ui->logTab->clear();
    ui->logTab->append("=== Сброс ===");
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

    ui->logTab->append(QString("[День %1] Начало дня").arg(day));

    // TODO: тут будут вызовы simulation_step(day)
    // Пока — заглушка с наблюдаемыми событиями:
    int expired = 0;
    for (const auto& b : warehouse.inventory)
        if (b.expiration_day < day) expired++;
    if (expired > 0)
        ui->logTab->append(QString("  списано просроченных партий: %1").arg(expired));

    ui->logTab->append(QString("  всего партий на складе: %1")
                           .arg(warehouse.inventory.size()));

    refreshAllTabs();
}

void MainWindow::refreshAllTabs() {
    ui->stockTab->refresh();
}
