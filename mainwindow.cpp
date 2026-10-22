#include "mainwindow.h"
#include "back/data.h"
#include "front/animtab.h"
#include "front/courierstab.h"
#include "front/logtab.h"
#include "front/orderstab.h"
#include "front/requeststab.h"
#include "front/resultstab.h"
#include "front/stocktab.h"
#include "internal/simulation_cli.h"
#include <QtWidgets>
#include <limits>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("ФармДоставка · лаборатория моделирования");
  resize(1440, 940);
  setMinimumSize(1000, 720);
  setStyleSheet(R"(
        QWidget { font-family: 'Noto Sans'; font-size: 13px; color: #23364a; }
        QMainWindow, QWidget#shell { background: #f2f5f9; }
        QLabel#brand { font-size: 18px; font-weight: 700; color: #123b47; }
        QLabel#muted { color: #6d7e8f; }
        QFrame#card, QGroupBox { background: white; border: 1px solid #dfe7ee; border-radius: 12px; }
        QLabel#metric { font-size: 16px; font-weight: 700; color: #123b47; }
        QGroupBox { margin-top: 18px; padding: 18px 12px 12px; font-weight: 600; }
        QGroupBox::title { subcontrol-origin: margin; left: 16px; padding: 0 6px; }
        QPushButton { background: white; border: 1px solid #d6e0e8; border-radius: 7px; padding: 7px 12px; font-weight: 600; }
        QPushButton:hover { background: #e4f3f0; border-color: #64b9a6; }
        QPushButton:pressed { background: #c6e8df; }
        QPushButton#primary { background: #087f70; color: white; border-color: #087f70; }
        QPushButton#primary:hover { background: #06685d; }
        QPushButton:disabled { color: #9aa8b6; background: #edf1f5; border-color: #e0e6ed; }
        QTabWidget::pane { background: white; border: 1px solid #dfe7ee; border-radius: 10px; padding: 12px; }
        QTabBar::tab { border: none; background: transparent; padding: 8px 14px; color: #687b8f; margin-right: 3px; }
        QTabBar::tab:selected { color: #087f70; background: white; border-bottom: 3px solid #087f70; }
        QTabBar::tab:hover { color: #087f70; }
        QSpinBox, QDoubleSpinBox, QLineEdit { background: white; border: 1px solid #d6e0e8; border-radius: 5px; padding: 6px; min-height: 20px; }
        QAbstractSpinBox:focus, QLineEdit:focus { border-color: #087f70; }
        QTableView { background: white; alternate-background-color: #f5f8fb; border: none; gridline-color: #edf1f5; selection-background-color: #d9eee9; selection-color: #123b47; }
        QHeaderView::section { background: #edf3f7; border: none; padding: 10px; font-weight: 600; }
        QTextEdit { background: white; border: none; padding: 10px; }
        QProgressBar { border: none; border-radius: 3px; background: #dee8ed; max-height: 6px; }
        QProgressBar::chunk { background: #13a68c; border-radius: 3px; }
        QSlider::groove:horizontal { height: 5px; background: #d6e1e8; border-radius: 2px; }
        QSlider::handle:horizontal { width: 16px; margin: -6px 0; background: #087f70; border-radius: 8px; }
        QWidget#parametersPage { background: #f7fafc; }
        QScrollArea { border: none; background: white; }
    )");
  auto *shell = new QWidget;
  shell->setObjectName("shell");
  setCentralWidget(shell);
  auto *layout = new QVBoxLayout(shell);
  layout->setContentsMargins(16, 10, 16, 10);
  layout->setSpacing(8);
  auto *header = new QHBoxLayout;
  auto *brand = new QLabel("✚  ФармДоставка");
  brand->setObjectName("brand");
  header->addWidget(brand);
  header->addSpacing(14);
  auto card = [&](const QString &title, QLabel *&value, const QString &hint) {
    auto *frame = new QFrame;
    frame->setObjectName("card");
    frame->setToolTip(hint);
    auto *box = new QHBoxLayout(frame);
    box->setContentsMargins(10, 5, 10, 5);
    box->setSpacing(8);
    auto *label = new QLabel(title);
    label->setObjectName("muted");
    box->addWidget(label);
    value = new QLabel("—");
    value->setObjectName("metric");
    box->addWidget(value);
    header->addWidget(frame);
  };
  card("Доход", income, "Доход по выполненным доставкам");
  card("Прибыль", profit, "Прибыль с учётом себестоимости и списаний");
  card("Доставлено", deliveries, "Заказов за весь эксперимент");
  card("Склад", stock, "Доступных упаковок");
  header->addStretch();
  layout->addLayout(header);
  status = new QLabel("Настройте новый эксперимент");
  status->setObjectName("statusLabel");
  statusBar()->addPermanentWidget(status);
  exportButton = new QPushButton("Экспорт JSON");
  exportButton->setObjectName("exportBtn");
  auto *toolbar = new QHBoxLayout;
  start = new QPushButton("▶  Запустить");
  start->setObjectName("primary");
  start->setAccessibleName("Запустить");
  pause = new QPushButton("Ⅱ  Пауза");
  pause->setObjectName("pauseBtn");
  step = new QPushButton("Шаг +1 день");
  step->setObjectName("stepBtn");
  auto *reset = new QPushButton("Сбросить");
  reset->setObjectName("resetBtn");
  toolbar->addWidget(start);
  toolbar->addWidget(pause);
  toolbar->addWidget(step);
  toolbar->addWidget(reset);
  toolbar->addWidget(exportButton);
  toolbar->addStretch();
  dayLabel = new QLabel("День 0 / 15");
  dayLabel->setObjectName("dayLabel");
  toolbar->addWidget(dayLabel);
  toolbar->addSpacing(16);
  speedLabel = new QLabel;
  toolbar->addWidget(speedLabel);
  auto *speed = new QSlider(Qt::Horizontal);
  speed->setObjectName("speedSlider");
  speed->setRange(1, 10);
  speed->setFixedWidth(110);
  toolbar->addWidget(speed);
  layout->addLayout(toolbar);
  progress = new QProgressBar;
  progress->setTextVisible(false);
  layout->addWidget(progress);
  tabs = new QTabWidget;
  tabs->setObjectName("tabs");
  layout->addWidget(tabs, 1);
  animTab = new AnimTab;
  tabs->addTab(animTab, "Обзор");
  auto *scroll = new QScrollArea;
  scroll->setWidgetResizable(true);
  parametersPage = new QWidget;
  parametersPage->setObjectName("parametersPage");
  auto *settings = new QVBoxLayout(parametersPage);
  auto *intro = new QLabel("Параметры применяются при подготовке нового "
                           "эксперимента. Один шаг — один день.");
  intro->setWordWrap(true);
  settings->addWidget(intro);
  auto *grid = new QGridLayout;
  settings->addLayout(grid);
  auto group = [&](QString title, int row, int col) {
    auto *g = new QGroupBox(title);
    auto *f = new QFormLayout(g);
    f->setVerticalSpacing(10);
    grid->addWidget(g, row, col);
    return f;
  };
  auto integer = [&](QFormLayout *f, QString id, QString title, int low,
                     int high, int value) {
    auto *s = new QSpinBox;
    s->setObjectName(id);
    s->setRange(low, high);
    s->setValue(value);
    integers[id] = s;
    f->addRow(title, s);
  };
  auto decimal = [&](QFormLayout *f, QString id, QString title, double high,
                     double value, QString suffix) {
    auto *s = new QDoubleSpinBox;
    s->setObjectName(id);
    s->setRange(0, high);
    s->setValue(value);
    s->setSuffix(suffix);
    decimals[id] = s;
    f->addRow(title, s);
  };
  auto *general = group("01 / Эксперимент", 0, 0);
  integer(general, "daysSpin", "Период, дней (N)", 10, 25, 15);
  integer(general, "couriersSpin", "Курьеры (M)", 3, 9, 5);
  integer(general, "medicinesSpin", "Виды лекарств (K)", 15, 35, 25);
  integer(general, "ordersPerDaySpin", "Предел заказов в день", 0, 100000, 180);
  integer(general, "regularCustomersSpin", "Постоянные покупатели", 0, 100000,
          10);
  seed = new QLineEdit("5489");
  seed->setObjectName("seedEdit");
  seed->setToolTip(
      "Одинаковое зерно и параметры позволяют повторить эксперимент.");
  general->addRow("Зерно случайности", seed);
  auto *pricing = group("02 / Цены и скидки", 0, 1);
  decimal(pricing, "markupSpin", "Наценка", 10000, 25, " %");
  decimal(pricing, "cardDiscountSpin", "Дисконтная карта", 100, 5, " %");
  decimal(pricing, "bigOrderDiscountSpin", "Покупка свыше 1 000 ₽", 100, 3,
          " %");
  decimal(pricing, "regularDiscountSpin", "Постоянный покупатель", 100, 5,
          " %");
  decimal(pricing, "maxDiscountSpin", "Предел общей скидки", 9, 9, " %");
  auto *warehouseForm = group("03 / Склад и пополнение", 1, 0);
  integer(warehouseForm, "initialCountSpin", "Упаковок каждого вида", 0,
          1000000, 80);
  integer(warehouseForm, "thresholdSpin", "Порог пополнения", 0, 1000000, 20);
  integer(warehouseForm, "expiryDaysSpin", "Дней до уценки", 0, 1000000, 30);
  decimal(warehouseForm, "discountedProbSpin", "Выбор уценённого товара", 100,
          75, " %");
  auto *finances = group("04 / Начальные условия", 1, 1);
  decimal(finances, "capitalSpin", "Начальный капитал", 1e12, 1000000, " ₽");
  auto *fileRow = new QWidget;
  auto *fileLayout = new QHBoxLayout(fileRow);
  fileLayout->setContentsMargins(0, 0, 0, 0);
  stockFile = new QLineEdit;
  stockFile->setObjectName("stockFileEdit");
  stockFile->setPlaceholderText("Автоматический начальный склад");
  stockFile->setClearButtonEnabled(true);
  auto *browse = new QPushButton("Выбрать…");
  fileLayout->addWidget(stockFile);
  fileLayout->addWidget(browse);
  finances->addRow("Склад CSV", fileRow);
  auto *note =
      new QLabel("CSV: medicine_id,count,expiration_day,unit_price\nФайл "
                 "полностью заменяет автоматически созданные партии.");
  note->setWordWrap(true);
  finances->addRow(note);
  generate = new QPushButton("Подготовить эксперимент");
  generate->setObjectName("generateBtn");
  settings->addWidget(generate, 0, Qt::AlignLeft);
  settings->addStretch();
  scroll->setWidget(parametersPage);
  tabs->addTab(scroll, "Параметры");
  stockTab = new StockTab;
  tabs->addTab(stockTab, "Склад");
  ordersTab = new OrdersTab;
  tabs->addTab(ordersTab, "Заказы");
  couriersTab = new CouriersTab;
  tabs->addTab(couriersTab, "Курьеры");
  requestsTab = new RequestsTab;
  tabs->addTab(requestsTab, "Поставки");
  resultsTab = new ResultsTab;
  tabs->addTab(resultsTab, "Результаты");
  logTab = new LogTab;
  tabs->addTab(logTab, "Журнал");
  for (auto *table : findChildren<QTableView *>()) {
    table->verticalHeader()->hide();
    table->verticalHeader()->setDefaultSectionSize(38);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setShowGrid(false);
  }
  timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &MainWindow::doStep);
  connect(speed, &QSlider::valueChanged, this, [=](int v) {
    timer->setInterval(2200 - v * 200);
    speedLabel->setText(
        QString("%1 с / день").arg(timer->interval() / 1000.0, 0, 'f', 1));
  });
  speed->setValue(5);
  connect(start, &QPushButton::clicked, this, [=] {
    if ((!initialized || day >= totalDays) && !initialize())
      return;
    setRunning(true);
  });
  connect(pause, &QPushButton::clicked, this, [=] { setRunning(false); });
  connect(step, &QPushButton::clicked, this, [=] {
    setRunning(false);
    doStep();
  });
  connect(generate, &QPushButton::clicked, this, [=] {
    if (initialize())
      tabs->setCurrentIndex(0);
  });
  connect(reset, &QPushButton::clicked, this, [=] { initialize(); });
  connect(browse, &QPushButton::clicked, this, [=] {
    auto path = QFileDialog::getOpenFileName(this, "Начальный склад", {},
                                             "CSV (*.csv);;Все файлы (*)");
    if (!path.isEmpty())
      stockFile->setText(path);
  });
  connect(exportButton, &QPushButton::clicked, this, [=] {
    const auto path = QFileDialog::getSaveFileName(
        this, "Экспорт эксперимента", "experiment.json", "JSON (*.json)");
    if (path.isEmpty())
      return;
    try {
      model_detail::write_json_report(path.toStdString());
      statusBar()->showMessage("Результаты сохранены: " + path, 6000);
    } catch (const std::exception &e) {
      QMessageBox::warning(this, "Экспорт не выполнен",
                           QString::fromUtf8(e.what()));
    }
  });
  statusBar()->showMessage(
      "Доставка на следующий день · целевая нагрузка 7–15 заказов на курьера");
  updateControls();
}

SimulationParameters MainWindow::buildParameters() const {
  SimulationParameters p;
  p.days = integers["daysSpin"]->value();
  p.couriers = integers["couriersSpin"]->value();
  p.medicine_count = integers["medicinesSpin"]->value();
  p.orders_per_day = integers["ordersPerDaySpin"]->value();
  p.regular_customers = integers["regularCustomersSpin"]->value();
  p.initial_count = integers["initialCountSpin"]->value();
  p.restock_threshold = integers["thresholdSpin"]->value();
  p.markdown_days = integers["expiryDaysSpin"]->value();
  p.markup_percent = decimals["markupSpin"]->value();
  p.card_discount = decimals["cardDiscountSpin"]->value();
  p.large_purchase_discount = decimals["bigOrderDiscountSpin"]->value();
  p.regular_discount = decimals["regularDiscountSpin"]->value();
  p.maximum_discount = decimals["maxDiscountSpin"]->value();
  p.initial_capital = decimals["capitalSpin"]->value();
  p.discounted_probability = decimals["discountedProbSpin"]->value() / 100.0;
  p.initial_stock_file = stockFile->text().trimmed().toStdString();
  bool ok = false;
  auto value = seed->text().toULongLong(&ok);
  if (!ok || value > std::numeric_limits<std::uint32_t>::max())
    throw std::invalid_argument(
        "Зерно должно быть целым числом от 0 до 4294967295.");
  p.seed = static_cast<std::uint32_t>(value);
  return p;
}

bool MainWindow::initialize() {
  setRunning(false);
  try {
    const auto p = buildParameters();
    p.validate();
    configure_simulation(p);
    init_simulation();
    totalDays = p.days;
  } catch (const std::exception &e) {
    reportError(QString::fromUtf8(e.what()));
    return false;
  }
  initialized = true;
  failed = false;
  day = 0;
  logTab->clear();
  logTab->append("Эксперимент подготовлен. В первый день заказы формируются, "
                 "доставка начинается со второго.");
  animTab->resetAnimation();
  refresh();
  updateControls();
  return true;
}
void MainWindow::setRunning(bool running) {
  if (running)
    timer->start();
  else
    timer->stop();
  animTab->setRunning(running);
  updateControls();
}
void MainWindow::reportError(const QString &message) {
  failed = true;
  setRunning(false);
  status->setText("Ошибка · исправьте параметры и подготовьте заново");
  logTab->append("Ошибка: " + message);
  statusBar()->showMessage(message);
  tabs->setCurrentWidget(logTab);
}
void MainWindow::doStep() {
  if (!initialized && !initialize())
    return;
  if (failed || day >= totalDays)
    return;
  try {
    const int next = day + 1;
    manage_warehouse(next);
    generate_daily_events(next);
    process_daily_order(next);
    cleanup_random_customers();
    day = next;
  } catch (const std::exception &e) {
    reportError(QString::fromUtf8(e.what()));
    return;
  }
  const auto &d = simulation_result().days.back();
  logTab->append(QString("День %1 · новых заказов: %2 · доставлено: %3 · "
                         "заявок поставщику: %4 · поставок прибыло: %5")
                     .arg(day)
                     .arg(d.received_orders)
                     .arg(d.delivered_orders)
                     .arg(d.created_supplies)
                     .arg(d.arrived_supplies));
  refresh();
  if (day >= totalDays)
    setRunning(false);
  updateControls();
}
void MainWindow::refresh() {
  stockTab->refresh();
  ordersTab->refresh();
  couriersTab->refresh();
  requestsTab->refresh();
  resultsTab->refresh();
  const auto &r = simulation_result();
  auto money = [](double v) {
    return QLocale(QLocale::Russian).toString(v, 'f', 0) + " ₽";
  };
  income->setText(money(r.totals.income));
  profit->setText(money(r.profit));
  deliveries->setText(QString::number(r.completed_orders.size()));
  qint64 count = 0;
  for (const auto &b : warehouse.inventory)
    count += b.count;
  stock->setText(QLocale(QLocale::Russian).toString(count));
  animTab->updateState(
      day, count,
      r.days.empty() ? std::vector<int>(simulation_parameters().couriers, 0)
                     : r.days.back().courier_load,
      r.days.empty() ? 0 : r.days.back().created_supplies);
}
void MainWindow::updateControls() {
  const bool running = timer->isActive();
  parametersPage->setEnabled(!running);
  start->setEnabled(!running && !failed);
  pause->setEnabled(running);
  step->setEnabled(!running && !failed && (!initialized || day < totalDays));
  start->setText(day >= totalDays ? "↻  Повторить" : "▶  Запустить");
  exportButton->setEnabled(initialized && !failed);
  dayLabel->setText(QString("День %1 / %2").arg(day).arg(totalDays));
  progress->setRange(0, totalDays);
  progress->setValue(day);
  if (!failed)
    status->setText(running            ? "●  Эксперимент идёт"
                    : day >= totalDays ? "✓  Эксперимент завершён"
                    : initialized      ? "●  Готов к следующему шагу"
                                       : "Настройте новый эксперимент");
}
