#pragma once
#include "back/simulation.h"
#include <QMainWindow>
#include <QMap>
class QTimer;
class QTabWidget;
class QPushButton;
class QLabel;
class QProgressBar;
class QSpinBox;
class QDoubleSpinBox;
class QLineEdit;
class StockTab;
class OrdersTab;
class CouriersTab;
class RequestsTab;
class ResultsTab;
class AnimTab;
class LogTab;

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget *parent = nullptr);

private:
  QTimer *timer;
  QTabWidget *tabs;
  QWidget *parametersPage;
  QPushButton *start, *pause, *step, *generate, *exportButton;
  QLabel *status, *dayLabel, *income, *profit, *deliveries, *stock, *speedLabel;
  QProgressBar *progress;
  QMap<QString, QSpinBox *> integers;
  QMap<QString, QDoubleSpinBox *> decimals;
  QLineEdit *stockFile, *seed;
  StockTab *stockTab;
  OrdersTab *ordersTab;
  CouriersTab *couriersTab;
  RequestsTab *requestsTab;
  ResultsTab *resultsTab;
  AnimTab *animTab;
  LogTab *logTab;
  int day = 0;
  int totalDays = 15;
  bool initialized = false;
  bool failed = false;
  SimulationParameters buildParameters() const;
  bool initialize();
  void doStep();
  void refresh();
  void updateControls();
  void setRunning(bool running);
  void reportError(const QString &message);
};
