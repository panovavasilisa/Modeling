#pragma once
#include <QElapsedTimer>
#include <QTimer>
#include <QWidget>
#include <vector>
class QGraphicsScene;
class QGraphicsItem;
class QGraphicsView;
class QLabel;
class QSlider;

// Город служит визуализацией модели; количество зданий не меняет поток заказов.
class AnimTab : public QWidget {
  Q_OBJECT
public:
  explicit AnimTab(QWidget *parent = nullptr);
  void updateState(int day, qint64 stockCount, const std::vector<int> &loads,
                   int newRequests);
  void resetAnimation();
  void setRunning(bool running);

protected:
  void showEvent(QShowEvent *) override;
  void hideEvent(QHideEvent *) override;

private:
  void buildCity();
  void rebuildVehicles();
  void moveVehicles();
  void refreshLabels();
  QTimer timer;
  QElapsedTimer elapsed;
  QGraphicsScene *scene;
  QGraphicsView *view;
  QSlider *buildingSlider, *zoomSlider;
  QLabel *buildingLabel, *stateLabel, *summaryLabel, *courierLabel, *zoomLabel;
  std::vector<QGraphicsItem *> vehicles;
  std::vector<int> loads;
  int currentDay = 0;
  qint64 totalStock = 0;
  int requests = 0;
  double distance = 0;
  bool running = false;
};
