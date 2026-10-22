#include "front/animtab.h"
#include "front/requeststab.h"
#include "internal/simulation_cli.h"
#include "mainwindow.h"
#include <QtTest>
#include <QtWidgets>

class FrontendTests : public QObject {
  Q_OBJECT
private slots:
  void cityControls() {
    AnimTab map;
    map.resize(1100, 550);
    map.updateState(5, 800, {7, 8, 9, 10, 11, 12, 13, 14, 15}, 3);
    map.show();
    QTest::qWait(40);
    auto *buildings = map.findChild<QSlider *>("buildingCountSlider");
    auto *zoom = map.findChild<QSlider *>("mapZoomSlider");
    auto *view = map.findChild<QGraphicsView *>("cityView");
    QVERIFY(buildings);
    QVERIFY(zoom);
    QVERIFY(view);
    const auto daysBefore = simulation_result().days.size();
    for (int count : {60, 240, 376}) {
      buildings->setValue(count);
      int actual = 0;
      QMap<int, QSet<int>> blockTypes;
      QSet<int> poolTypes;
      for (auto *item : view->scene()->items()) {
        if (item->data(0).toString() == "city-building") {
          ++actual;
          const int block = item->data(2).toInt(), type = item->data(1).toInt();
          QVERIFY(type >= 0 && type < 20);
          QVERIFY(!blockTypes[block].contains(type));
          blockTypes[block].insert(type);
          poolTypes.insert(type);
          QVERIFY(!item->sceneBoundingRect().intersects(
              QRectF(682, 272, 376, 556)));
        }
      }
      QCOMPARE(actual, count);
      if (count == 376)
        QCOMPARE(poolTypes.size(), 20);
      QCOMPARE(simulation_result().days.size(), daysBefore);
    }
    for (int factor : {100, 200, 300}) {
      zoom->setValue(factor);
      QVERIFY(qAbs(view->transform().m11() - view->transform().m22()) < 1e-9);
      QVERIFY(view->transform().m11() > 0);
    }
    map.setRunning(true);
    for (int i = 0; i < 6; ++i) {
      buildings->setValue(i % 2 ? 376 : 60);
      QTest::qWait(40);
      // Все машины остаются в дорожной сети, за пределами центрального парка.
      int cars = 0;
      for (auto *item : view->scene()->items())
        if (item->zValue() == 10) {
          ++cars;
          QVERIFY(!QRectF(682, 272, 376, 556).contains(item->pos()));
          const auto pos = item->pos();
          double dx = 1e9, dy = 1e9;
          for (int col = 0; col <= 10; ++col)
            dx = qMin(dx, qAbs(pos.x() - (100 + 140 * col)));
          for (int row = 0; row <= 6; ++row)
            dy = qMin(dy, qAbs(pos.y() - (100 + 150 * row)));
          QVERIFY(qMin(dx, dy) < 15);
        }
      QCOMPARE(cars, 9);
    }
    map.setRunning(false);
    buildings->setValue(376);
    zoom->setValue(160);
    map.grab().save("/tmp/pharm-city-close.png");
    map.resize(740, 360);
    zoom->setValue(100);
    QTest::qWait(30);
    QVERIFY(qAbs(view->transform().m11() - view->transform().m22()) < 1e-9);
    map.grab().save("/tmp/pharm-city-small.png");
  }
  void workflow() {
    QApplication::setStyle("Fusion");
    MainWindow w;
    w.show();
    QTest::qWait(80);
    auto button = [&](const char *name) {
      return w.findChild<QPushButton *>(name);
    };
    auto *tabs = w.findChild<QTabWidget *>("tabs");
    QVERIFY(button("stepBtn"));
    QVERIFY(button("generateBtn"));
    QVERIFY(!button("exportBtn")->isEnabled());
    button("stepBtn")->click();
    QCOMPARE(simulation_result().days.size(), size_t(1));
    QCOMPARE(simulation_result().completed_orders.size(), size_t(0));
    button("stepBtn")->click();
    QCOMPARE(simulation_result().days.size(), size_t(2));
    QVERIFY(!simulation_result().completed_orders.empty());
    // Пауза останавливает и отсчёт дней, и анимацию маршрутов.
    button("primary")->click();
    QVERIFY(button("pauseBtn")->isEnabled());
    QTest::qWait(90);
    auto frame1 = w.findChild<AnimTab *>()->grab().toImage();
    QTest::qWait(120);
    auto frame2 = w.findChild<AnimTab *>()->grab().toImage();
    QVERIFY(frame1 != frame2);
    button("pauseBtn")->click();
    auto stopped = w.findChild<AnimTab *>()->grab().toImage();
    QTest::qWait(120);
    QCOMPARE(w.findChild<AnimTab *>()->grab().toImage(), stopped);
    QCOMPARE(simulation_result().days.size(), size_t(2));
    button("resetBtn")->click();
    QCOMPARE(simulation_result().days.size(), size_t(0));
    QCOMPARE(simulation_result().completed_orders.size(), size_t(0));
    for (int i = 0; i < 15; ++i)
      button("stepBtn")->click();
    QCOMPARE(simulation_result().days.size(), size_t(15));
    QVERIFY(!button("stepBtn")->isEnabled());
    QVERIFY(!button("pauseBtn")->isEnabled());
    // В поставках отображаются названия всех лекарств, включая ID выше числа
    // видов.
    auto *table = w.findChild<RequestsTab *>()->findChild<QTableView *>();
    for (int i = 0; i < table->model()->rowCount(); ++i)
      QVERIFY(table->model()->index(i, 1).data().toString() != "?");
    QTemporaryDir output;
    auto path = output.filePath("report.json");
    model_detail::write_json_report(path.toStdString());
    QFile json(path);
    QVERIFY(json.open(QIODevice::ReadOnly));
    auto report = QJsonDocument::fromJson(json.readAll());
    QVERIFY(report.isObject());
    QCOMPARE(report["days"].toArray().size(), 15);
    tabs->setCurrentIndex(0);
    QTest::qWait(60);
    w.grab().save("/tmp/pharm-overview.png");
    tabs->setCurrentIndex(1);
    QTest::qWait(60);
    w.grab().save("/tmp/pharm-parameters.png");
    w.resize(1000, 720);
    QTest::qWait(60);
    w.grab().save("/tmp/pharm-small.png");
    w.resize(1440, 940);
    tabs->setCurrentIndex(6);
    QTest::qWait(60);
    w.grab().save("/tmp/pharm-results.png");
    // Неверные параметры не запускают таймер; после исправления возможен новый
    // запуск.
    w.findChild<QLineEdit *>("seedEdit")->setText("-1");
    button("generateBtn")->click();
    QVERIFY(!button("primary")->isEnabled());
    QVERIFY(!button("exportBtn")->isEnabled());
    w.findChild<QLineEdit *>("seedEdit")->setText("42");
    button("generateBtn")->click();
    QVERIFY(button("primary")->isEnabled());
    QCOMPARE(simulation_result().days.size(), size_t(0));
    w.findChild<QSpinBox *>("ordersPerDaySpin")->setValue(0);
    button("generateBtn")->click();
    button("stepBtn")->click();
    QCOMPARE(simulation_result().days.back().received_orders, 0);
    // Автоматический режим доходит до N и останавливается самостоятельно.
    w.findChild<QSpinBox *>("daysSpin")->setValue(10);
    w.findChild<QSlider *>("speedSlider")->setValue(10);
    button("generateBtn")->click();
    button("primary")->click();
    QTRY_COMPARE_WITH_TIMEOUT(simulation_result().days.size(), size_t(10),
                              4000);
    QVERIFY(!button("pauseBtn")->isEnabled());
    QVERIFY(!button("stepBtn")->isEnabled());
    // Ошибка чтения CSV допускает исправление и повторную инициализацию.
    w.findChild<QLineEdit *>("stockFileEdit")
        ->setText(output.filePath("missing.csv"));
    button("generateBtn")->click();
    QVERIFY(!button("exportBtn")->isEnabled());
    QFile csv(output.filePath("stock.csv"));
    QVERIFY(csv.open(QIODevice::WriteOnly));
    csv.write("medicine_id,count,expiration_day,unit_price\n0,40,90,100\n");
    csv.close();
    w.findChild<QLineEdit *>("stockFileEdit")->setText(csv.fileName());
    button("generateBtn")->click();
    QVERIFY(button("exportBtn")->isEnabled());
    QCOMPARE(simulation_result().initial_packages, qint64(40));
  }
};
QTEST_MAIN(FrontendTests)
#include "frontend_tests.moc"
