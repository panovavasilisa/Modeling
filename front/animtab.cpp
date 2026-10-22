#include "animtab.h"
#include <QtWidgets>
#include <algorithm>
#include <cmath>

namespace {
const double streetsX[] = {100, 230,  365,  520,  660, 800,
                           940, 1080, 1220, 1370, 1500};
const double streetsY[] = {100, 250, 410, 560, 700, 850, 1000};
// Двадцать типов: разные габариты, высота, материалы и силуэты крыш.
struct BuildingStyle {
  double width, depth, height;
  const char *color;
  int roof;
};
const BuildingStyle buildingPool[] = {
    {15, 19, 8, "#c59377", 0},  {17, 16, 12, "#aab8bf", 1},
    {13, 18, 22, "#6f96a7", 2}, {17, 24, 7, "#cebb9a", 3},
    {15, 21, 15, "#af9e92", 4}, {12, 16, 25, "#819ba8", 2},
    {17, 18, 10, "#b27c69", 1}, {14, 25, 8, "#a4b6a2", 0},
    {16, 17, 19, "#97a9bf", 4}, {17, 23, 9, "#d0a991", 3},
    {14, 19, 14, "#91a5a9", 1}, {12, 17, 23, "#788ba6", 2},
    {17, 24, 6, "#ceb992", 0},  {15, 18, 17, "#baaaa3", 4},
    {16, 22, 11, "#ac8b7b", 3}, {13, 16, 20, "#8da3b8", 2},
    {17, 20, 8, "#b8c3b6", 1},  {14, 24, 12, "#a69589", 0},
    {16, 18, 16, "#a0afb7", 4}, {17, 21, 10, "#ca9e84", 3}};
const char *colors[] = {"#0c9f99", "#507ee0", "#e6a044", "#ac72d0", "#dc759b",
                        "#59a46e", "#5aa7c4", "#c3a54c", "#9392db"};
class CityView : public QGraphicsView {
public:
  explicit CityView(QGraphicsScene *scene) : QGraphicsView(scene) {
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setFrameShape(QFrame::NoFrame);
    setBackgroundBrush(QColor("#e9eff0"));
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
  }
  double zoom = 1.0;
  void fitCity() {
    resetTransform();
    const double fit = qMin((viewport()->width() - 8) / 1600.0,
                            (viewport()->height() - 8) / 1100.0);
    const double factor = qMax(0.01, fit) * zoom;
    scale(factor, factor);
  }

protected:
  void resizeEvent(QResizeEvent *e) override {
    QGraphicsView::resizeEvent(e);
    fitCity();
  }
};

// Периметры проходят по уличной сетке, не пересекают парк и возвращаются к
// складу.
QPainterPath routeFor(int index) {
  const double right = index % 3 == 0 ? 520 : index % 3 == 1 ? 1220 : 1500;
  const double top = index % 3 == 0 ? 250 : 100;
  const double bottom = index % 3 == 0 ? 850 : 1000;
  QPainterPath p(QPointF(100, 625));
  p.lineTo(100, top + 18);
  p.quadTo(100, top, 118, top);
  p.lineTo(right - 18, top);
  p.quadTo(right, top, right, top + 18);
  p.lineTo(right, bottom - 18);
  p.quadTo(right, bottom, right - 18, bottom);
  p.lineTo(118, bottom);
  p.quadTo(100, bottom, 100, bottom - 18);
  p.lineTo(100, 625);
  return p;
}
} // namespace

AnimTab::AnimTab(QWidget *parent) : QWidget(parent) {
  setMinimumSize(720, 300);
  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(10);
  auto *controls = new QHBoxLayout;
  auto *title = new QLabel("Городская доставка");
  title->setStyleSheet("font-size: 17px; font-weight: 700; color: #254a50;");
  controls->addWidget(title);
  controls->addStretch();
  buildingLabel = new QLabel;
  buildingLabel->setMinimumWidth(95);
  controls->addWidget(buildingLabel);
  buildingSlider = new QSlider(Qt::Horizontal);
  buildingSlider->setObjectName("buildingCountSlider");
  buildingSlider->setRange(60, 376);
  buildingSlider->setValue(240);
  buildingSlider->setFixedWidth(135);
  controls->addWidget(buildingSlider);
  zoomLabel = new QLabel;
  zoomLabel->setMinimumWidth(95);
  controls->addWidget(zoomLabel);
  zoomSlider = new QSlider(Qt::Horizontal);
  zoomSlider->setObjectName("mapZoomSlider");
  zoomSlider->setRange(100, 300);
  zoomSlider->setValue(100);
  zoomSlider->setFixedWidth(100);
  controls->addWidget(zoomSlider);
  layout->addLayout(controls);
  auto *body = new QHBoxLayout;
  body->setSpacing(12);
  layout->addLayout(body, 1);
  scene = new QGraphicsScene(this);
  scene->setSceneRect(0, 0, 1600, 1100);
  view = new CityView(scene);
  view->setObjectName("cityView");
  body->addWidget(view, 1);
  auto *panel = new QFrame;
  panel->setObjectName("card");
  panel->setMinimumWidth(170);
  auto *side = new QVBoxLayout(panel);
  side->setContentsMargins(14, 14, 14, 14);
  stateLabel = new QLabel;
  stateLabel->setWordWrap(true);
  side->addWidget(stateLabel);
  auto *heading = new QLabel("КУРЬЕРЫ / ЗАКАЗЫ");
  heading->setStyleSheet("font-size: 11px; color: #82969a;");
  side->addWidget(heading);
  courierLabel = new QLabel;
  courierLabel->setTextFormat(Qt::RichText);
  courierLabel->setWordWrap(true);
  side->addWidget(courierLabel);
  side->addStretch();
  auto *note = new QLabel(
      "Норма: 7–15 в день\n\nМасштабируйте карту и перетаскивайте её мышью.");
  note->setWordWrap(true);
  note->setStyleSheet("color:#83979a;font-size:11px;");
  side->addWidget(note);
  auto *panelScroll = new QScrollArea;
  panelScroll->setWidgetResizable(true);
  panelScroll->setFixedWidth(200);
  panelScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  panelScroll->setFrameShape(QFrame::NoFrame);
  panelScroll->setWidget(panel);
  body->addWidget(panelScroll);
  summaryLabel = new QLabel;
  summaryLabel->setWordWrap(true);
  summaryLabel->setStyleSheet("color:#7b8f94;font-size:11px;");
  layout->addWidget(summaryLabel);
  connect(buildingSlider, &QSlider::valueChanged, this,
          [this] { buildCity(); });
  connect(zoomSlider, &QSlider::valueChanged, this, [this](int value) {
    auto *city = static_cast<CityView *>(view);
    city->zoom = value / 100.0;
    city->fitCity();
    zoomLabel->setText(QString("Масштаб %1%").arg(value));
  });
  timer.setInterval(30);
  connect(&timer, &QTimer::timeout, this, [this] {
    const double seconds = qMin(0.1, elapsed.restart() / 1000.0);
    distance += seconds * 100.0;
    moveVehicles();
  });
  zoomLabel->setText("Масштаб 100%");
  buildCity();
  refreshLabels();
}
void AnimTab::buildCity() {
  vehicles.clear();
  scene->clear();
  buildingLabel->setText(QString("Зданий: %1").arg(buildingSlider->value()));
  auto rect = [&](QRectF r, QColor c, double z = 0) {
    auto *item = scene->addRect(r, Qt::NoPen, c);
    item->setZValue(z);
    return item;
  };
  auto line = [&](QLineF l, QColor c, double width, double z = 0) {
    auto *item = scene->addLine(l, QPen(c, width, Qt::SolidLine, Qt::RoundCap));
    item->setZValue(z);
  };
  auto label = [&](QPointF pos, QString str, int size, QColor color,
                   double z = 1) {
    auto *t = scene->addSimpleText(str);
    QFont f("Noto Sans");
    f.setPixelSize(size);
    f.setBold(true);
    t->setFont(f);
    t->setBrush(color);
    t->setPos(pos);
    t->setZValue(z);
  };
  rect({0, 0, 1600, 1100}, "#d8e5e7");
  rect({57, 57, 1486, 986}, "#e6e7e0");
  // Сначала объединяем все дорожные полосы в единое покрытие.
  // Поэтому бордюры не проходят поперёк перекрёстков.
  std::vector<QLineF> roads;
  auto road = [&](QPointF a, QPointF b) { roads.emplace_back(a, b); };
  for (int col = 0; col <= 10; ++col) {
    double x = streetsX[col];
    if (col == 5 || col == 6) {
      road({x, 100}, {x, 250});
      road({x, 850}, {x, 1000});
    } else if (col == 1)
      road({x, 250}, {x, 1000});
    else if (col == 9)
      road({x, 100}, {x, 850});
    else
      road({x, 100}, {x, 1000});
  }
  for (int row = 0; row <= 6; ++row) {
    double y = streetsY[row];
    for (int col = 0; col < 10; ++col) {
      if (row >= 2 && row <= 4 && col >= 4 && col <= 6)
        continue;
      if ((row == 3 && col == 1) || (row == 2 && col == 8))
        continue;
      road({streetsX[col], y}, {streetsX[col + 1], y});
    }
  }
  QPainterPath centerlines;
  for (const auto &road : roads) {
    centerlines.moveTo(road.p1());
    centerlines.lineTo(road.p2());
  }
  QPainterPathStroker stroke;
  stroke.setCapStyle(Qt::RoundCap);
  stroke.setJoinStyle(Qt::RoundJoin);
  stroke.setWidth(43);
  scene->addPath(stroke.createStroke(centerlines).simplified(), Qt::NoPen,
                 QColor("#f2f0e7"));
  stroke.setWidth(30);
  scene->addPath(stroke.createStroke(centerlines).simplified(), Qt::NoPen,
                 QColor("#7e8c91"));
  // Разметка заканчивается перед пересечением, оставляя чистую площадку.
  for (const auto &road : roads) {
    const bool vertical = qAbs(road.dx()) < 1;
    const double begin = vertical ? road.y1() : road.x1(),
                 end = vertical ? road.y2() : road.x2();
    for (double d = begin + 25; d < end - 20; d += 13) {
      bool intersection = false;
      if (vertical) {
        for (double y : streetsY)
          if (qAbs(d - y) < 23)
            intersection = true;
      } else {
        for (double x : streetsX)
          if (qAbs(d - x) < 23)
            intersection = true;
      }
      if (!intersection)
        line(vertical ? QLineF(road.x1(), d, road.x1(), d + 6)
                      : QLineF(d, road.y1(), d + 6, road.y1()),
             "#d3d8d2", 1.3, .1);
    }
  }
  // Нерегулярные поляны, извилистые дорожки и естественный берег озера.
  QPainterPath park;
  park.addRoundedRect(QRectF(682, 272, 376, 556), 24, 24);
  scene->addPath(park, Qt::NoPen, QColor("#aac59a"));
  for (int i = 0; i < 7; ++i) {
    auto *glade = scene->addEllipse(
        710 + (i * 53) % 205, 295 + (i * 71) % 420, 110 + (i % 3) * 20,
        80 + (i % 2) * 40, Qt::NoPen, QColor(i % 2 ? "#b7cea2" : "#a3bf91"));
    Q_UNUSED(glade);
  }
  QPainterPath promenade;
  promenade.moveTo(682, 510);
  promenade.cubicTo(750, 495, 760, 440, 830, 478);
  promenade.cubicTo(940, 540, 875, 640, 990, 680);
  promenade.cubicTo(1045, 710, 990, 780, 1010, 828);
  scene->addPath(promenade,
                 QPen(QColor("#e9dec3"), 11, Qt::SolidLine, Qt::RoundCap));
  QPainterPath walk;
  walk.moveTo(850, 272);
  walk.cubicTo(720, 310, 720, 420, 800, 470);
  walk.cubicTo(880, 530, 760, 610, 745, 715);
  walk.cubicTo(730, 780, 850, 795, 880, 828);
  scene->addPath(walk, QPen(QColor("#e4ddbe"), 7, Qt::SolidLine, Qt::RoundCap));
  QPainterPath lake;
  lake.moveTo(845, 322);
  lake.cubicTo(900, 298, 923, 350, 980, 345);
  lake.cubicTo(1040, 345, 1022, 402, 979, 414);
  lake.cubicTo(949, 431, 951, 466, 905, 448);
  lake.cubicTo(852, 426, 813, 455, 805, 410);
  lake.cubicTo(783, 371, 816, 358, 845, 322);
  scene->addPath(lake, QPen(QColor("#d6d5ad"), 7), QColor("#80acaf"));
  QPainterPath shine;
  shine.moveTo(860, 339);
  shine.cubicTo(907, 327, 934, 369, 970, 361);
  scene->addPath(shine, QPen(QColor("#a1c1bf"), 2));
  // Площадь с фонтаном, скамейками и небольшими цветниками.
  scene->addEllipse(825, 569, 66, 54, Qt::NoPen, QColor("#dfd6ba"));
  scene->addEllipse(846, 582, 24, 24, QPen(QColor("#f0ead9"), 4),
                    QColor("#8eb5b4"));
  for (int i = 0; i < 4; ++i) {
    rect({810.0 + i * 26, 637, 17, 4}, "#9b8060", 1);
    rect({810.0 + i * 26, 642, 17, 2}, "#b89a71", 1);
  }
  auto tree = [&](double x, double y, double radius, int tint) {
    scene->addEllipse(x - radius + 3, y - radius + 4, radius * 2, radius * 2,
                      Qt::NoPen, QColor(47, 76, 49, 28));
    scene->addEllipse(x - radius, y - radius, radius * 2, radius * 2, Qt::NoPen,
                      QColor(tint % 3 == 0   ? "#6e946b"
                             : tint % 3 == 1 ? "#819e70"
                                             : "#90aa79"));
    scene->addEllipse(x - radius * .55, y - radius * .7, radius, radius,
                      Qt::NoPen, QColor("#a4bb86"));
  };
  QPainterPathStroker pathClearance;
  pathClearance.setWidth(28);
  auto paths = pathClearance.createStroke(promenade).united(
      pathClearance.createStroke(walk));
  for (int i = 0; i < 230; ++i) {
    double x = 699 + (i * 137) % 342, y = 289 + (i * 173) % 523;
    QPointF pos(x, y);
    if (lake.contains(pos) || paths.contains(pos) ||
        QRectF(810, 554, 110, 105).contains(pos))
      continue;
    // Группы деревьев чередуются с открытыми лужайками.
    if (x > 750 && x < 960 && y > 675 && y < 780 && i % 4)
      continue;
    tree(x, y, 5 + (i % 7), i);
  }
  // Укрупнённые кварталы и разные отступы убирают одинаковую клетчатую
  // застройку.
  struct Block {
    double x, y, w, h;
    int columns, rows;
  };
  std::vector<Block> blocks;
  for (int row = 0; row < 6; ++row)
    for (int col = 0; col < 10; ++col) {
      if (col >= 4 && col <= 6 && row >= 1 && row <= 4)
        continue;
      if (col == 0 && row == 3)
        continue;
      if ((row == 0 && col == 1) || (row == 5 && col == 9))
        continue;
      if ((row == 3 && col == 1) || (row == 2 && col == 8))
        continue;
      const bool tall = (row == 2 && col == 1) || (row == 1 && col == 8);
      const bool merged = (row == 0 && col == 0) || (row == 5 && col == 8);
      blocks.push_back({streetsX[col], streetsY[row],
                        streetsX[col + (merged ? 2 : 1)] - streetsX[col],
                        streetsY[row + (tall ? 2 : 1)] - streetsY[row],
                        merged ? 8 : 4, tall ? 4 : 2});
    }
  int count = 0;
  for (int slot = 0; slot < 16 && count < buildingSlider->value(); ++slot)
    for (size_t blockIndex = 0;
         blockIndex < blocks.size() && count < buildingSlider->value();
         ++blockIndex) {
      const auto &b = blocks[blockIndex];
      if (slot >= b.columns * b.rows)
        continue;
      const int type = (int(blockIndex) * 3 + slot * 7) % 20;
      const auto &style = buildingPool[type];
      const double cell = (b.w - 44) / b.columns;
      const double x = b.x + 23 + (slot % b.columns) * cell + (type % 3),
                   y = b.y + 24 + (slot / b.columns) * (b.h - 48) / b.rows +
                       (type % 4);
      const double w = qMin(style.width, cell - 3), depth = style.depth,
                   height = style.height;
      auto *building = new QGraphicsItemGroup;
      scene->addItem(building);
      building->setPos(x, y);
      building->setZValue(2 + y / 1100.0);
      building->setData(0, "city-building");
      building->setData(1, type);
      building->setData(2, int(blockIndex));
      auto face = [&](QPolygonF polygon, QColor color) {
        auto *item = new QGraphicsPolygonItem(polygon, building);
        item->setPen(Qt::NoPen);
        item->setBrush(color);
      };
      const QColor color(style.color);
      face({{2, depth},
            {w + 5, depth + 4},
            {w + 5, depth + height + 4},
            {3, depth + height + 3}},
           QColor(42, 53, 62, 35));
      face({{0, depth}, {w, depth}, {w, depth + height}, {0, depth + height}},
           color.darker(118));
      face({{w - 4, 3},
            {w, 0},
            {w, depth + height},
            {w - 4, depth + height - 3}},
           color.darker(145));
      face({{0, 0}, {w, 0}, {w, depth}, {0, depth}}, color.lighter(116));
      for (int floor = 0; floor < int(height / 5); ++floor)
        for (int win = 0; win < 2; ++win) {
          auto *window = new QGraphicsRectItem(
              2 + win * (w / 2), depth + 3 + floor * 5, 2.5, 2.5, building);
          window->setPen(Qt::NoPen);
          window->setBrush(QColor(type % 2 ? "#d4e4dd" : "#e2d8bb"));
        }
      if (style.roof == 1) {
        face({{0, depth * .5},
              {w * .5, -3},
              {w, depth * .5},
              {w * .5, depth + 2}},
             color.darker(108));
      } else if (style.roof == 2) {
        face({{3, 4}, {w - 3, 4}, {w - 3, depth - 4}, {3, depth - 4}},
             color.darker(108));
        auto *antenna = new QGraphicsLineItem(w / 2, depth / 2, w / 2,
                                              depth / 2 - 8, building);
        antenna->setPen(QPen(QColor("#d8e2df"), 1));
      } else if (style.roof == 3) {
        face({{0, 0},
              {w, 0},
              {w, 6},
              {w * .55, 6},
              {w * .55, depth},
              {0, depth}},
             color.darker(106));
      } else if (style.roof == 4) {
        face({{3, 3}, {w - 2, 3}, {w - 2, depth * .65}, {3, depth * .65}},
             color.darker(108));
        face({{5, 5}, {w - 4, 5}, {w - 4, depth * .4}, {5, depth * .4}},
             color.lighter(135));
      } else {
        face({{3, 4}, {w - 4, 4}, {w - 4, 9}, {3, 9}}, color.darker(112));
      }
      ++count;
    }
  // Склад и погрузочная площадка на западной авеню.
  rect({122, 574, 96, 100}, "#d0d6cc", .4);
  rect({128, 580, 84, 71}, "#226e69", .5);
  rect({134, 585, 72, 54}, "#418a7e", .6);
  label({149, 590}, "✚", 30, "#f4faf1", 1);
  label({127, 656}, "СКЛАД", 15, "#37615b", 1);
  line({100, 625, 127, 625}, "#a7b5ac", 23, .4);
  rebuildVehicles();
  static_cast<CityView *>(view)->fitCity();
}
void AnimTab::rebuildVehicles() {
  for (auto *v : vehicles) {
    scene->removeItem(v);
    delete v;
  }
  vehicles.clear();
  for (size_t i = 0; i < loads.size(); ++i) {
    auto *car = new QGraphicsItemGroup;
    scene->addItem(car);
    car->setZValue(10);
    auto part = [&](QRectF r, QColor c) {
      auto *item = new QGraphicsRectItem(r, car);
      item->setPen(Qt::NoPen);
      item->setBrush(c);
    };
    part({-12, -10, 7, 4}, "#304852");
    part({6, -10, 7, 4}, "#304852");
    part({-12, 6, 7, 4}, "#304852");
    part({6, 6, 7, 4}, "#304852");
    part({-16, -7, 32, 14}, colors[i % 9]);
    part({7, -5, 6, 10}, "#d8f1f0");
    part({-13, -5, 17, 10}, "#f7f9ee");
    auto *id = new QGraphicsSimpleTextItem(QString::number(i + 1), car);
    QFont f("Noto Sans");
    f.setPixelSize(10);
    f.setBold(true);
    id->setFont(f);
    id->setBrush(QColor(colors[i % 9]));
    id->setPos(-10, -7);
    car->setVisible(loads[i] > 0);
    vehicles.push_back(car);
  }
  moveVehicles();
}
void AnimTab::moveVehicles() {
  for (size_t i = 0; i < vehicles.size(); ++i) {
    const auto route = routeFor(static_cast<int>(i));
    const double t =
        route.percentAtLength(std::fmod(distance + i * 280.0, route.length()));
    vehicles[i]->setPos(route.pointAtPercent(t));
    vehicles[i]->setRotation(-route.angleAtPercent(t));
  }
}
void AnimTab::refreshLabels() {
  stateLabel->setText(QString("День %1 · %2")
                          .arg(currentDay)
                          .arg(running ? "В движении" : "Пауза"));
  QString list = "<table width='100%' cellspacing='5'>";
  for (size_t i = 0; i < loads.size(); ++i)
    list += QString("<tr><td style='color:%1'>●</td><td>Курьер %2</td><td "
                    "align='right'><b>%3</b></td></tr>")
                .arg(colors[i % 9])
                .arg(i + 1)
                .arg(loads[i]);
  list += "</table>";
  if (loads.empty())
    list = "Нажмите «Запустить», чтобы начать эксперимент.";
  courierLabel->setText(list);
  summaryLabel->setText(
      QString("Склад: %1 упак.  ·  Новых заявок: %2  ·  Здания — оформление "
              "карты, не число покупателей. Маршруты условные.")
          .arg(totalStock)
          .arg(requests));
}
void AnimTab::setRunning(bool value) {
  running = value;
  if (value && isVisible()) {
    elapsed.start();
    timer.start();
  } else
    timer.stop();
  refreshLabels();
}
void AnimTab::showEvent(QShowEvent *e) {
  QWidget::showEvent(e);
  static_cast<CityView *>(view)->fitCity();
  if (running) {
    elapsed.start();
    timer.start();
  }
}
void AnimTab::hideEvent(QHideEvent *e) {
  timer.stop();
  QWidget::hideEvent(e);
}
void AnimTab::resetAnimation() {
  distance = 0;
  currentDay = 0;
  totalStock = 0;
  requests = 0;
  loads.clear();
  rebuildVehicles();
  refreshLabels();
}
void AnimTab::updateState(int d, qint64 s, const std::vector<int> &l, int r) {
  currentDay = d;
  totalStock = s;
  requests = r;
  loads = l;
  rebuildVehicles();
  refreshLabels();
}
