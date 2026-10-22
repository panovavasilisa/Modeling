#include "resultstab.h"
#include "back/simulation.h"
#include <QtWidgets>
#include <algorithm>

namespace {
class DeliveryChart : public QWidget {
public:
  explicit DeliveryChart(QWidget *parent = nullptr) : QWidget(parent) {
    setMinimumHeight(200);
  }
  void paintEvent(QPaintEvent *) override {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor("#f5f9fb"));
    p.setPen(QColor("#27485b"));
    p.drawText(22, 26, "Заказы по дням");
    const auto &days = simulation_result().days;
    if (days.empty()) {
      p.drawText(rect(), Qt::AlignCenter, "График появится после первого шага");
      return;
    }
    QRectF area(48, 50, width() - 70, height() - 83);
    int maxValue = 1;
    for (const auto &d : days)
      maxValue = std::max({maxValue, d.received_orders, d.delivered_orders});
    for (int i = 0; i <= 4; ++i) {
      double y = area.bottom() - area.height() * i / 4;
      p.setPen(QColor("#dfe8ef"));
      p.drawLine(QPointF(area.left(), y), QPointF(area.right(), y));
      p.setPen(QColor("#738697"));
      p.drawText(QRectF(0, y - 9, 40, 20), Qt::AlignRight,
                 QString::number(maxValue * i / 4));
    }
    const double step = area.width() / days.size();
    for (size_t i = 0; i < days.size(); ++i) {
      const double x = area.left() + step * i;
      const double bw = qMin(16.0, step * 0.3);
      p.fillRect(QRectF(x + step * .15,
                        area.bottom() -
                            area.height() * days[i].received_orders / maxValue,
                        bw, area.height() * days[i].received_orders / maxValue),
                 QColor("#a1bfd5"));
      p.fillRect(QRectF(x + step * .5,
                        area.bottom() -
                            area.height() * days[i].delivered_orders / maxValue,
                        bw,
                        area.height() * days[i].delivered_orders / maxValue),
                 QColor("#0d947c"));
      p.setPen(QColor("#738697"));
      p.drawText(QRectF(x, area.bottom() + 7, step, 20), Qt::AlignCenter,
                 QString::number(days[i].day));
    }
    p.setPen(QColor("#87a8c1"));
    p.drawText(width() - 275, 26, "● поступило");
    p.setPen(QColor("#0d947c"));
    p.drawText(width() - 145, 26, "● доставлено");
  }
};
} // namespace
ResultsTab::ResultsTab(QWidget *parent) : QWidget(parent) {
  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  chart = new DeliveryChart;
  layout->addWidget(chart, 1);
  text = new QTextEdit;
  text->setReadOnly(true);
  layout->addWidget(text, 1);
}
void ResultsTab::refresh() {
  const auto &r = simulation_result();
  const auto money = [](double v) {
    return QLocale(QLocale::Russian).toString(v, 'f', 2) + " ₽";
  };
  QString html = "<h2>Финансы и выполнение заказов</h2><table width='100%' "
                 "cellspacing='8'>";
  auto row = [&](QString label, QString value) {
    html += "<tr><td style='color:#61798b'>" + label + "</td><td><b>" + value +
            "</b></td></tr>";
  };
  row("Доход / прибыль", money(r.totals.income) + " / " + money(r.profit));
  row("Закупки / потери от списания", money(r.totals.purchase_expenses) +
                                          " / " +
                                          money(r.totals.write_off_losses));
  row("Денежный баланс", money(r.cash_balance));
  row("Стоимость доступного запаса", money(r.available_stock_cost));
  row("Доставлено / ожидает доставки / в очереди",
      QString("%1 / %2 / %3")
          .arg(r.completed_orders.size())
          .arg(r.pending_deliveries)
          .arg(r.pending_orders));
  row("Упаковки: начальный запас / поставлено / списано",
      QString("%1 / %2 / %3")
          .arg(r.initial_packages)
          .arg(r.supplied_packages)
          .arg(r.written_off_packages));
  html += "</table><p style='color:#738697'>Доход учитывается при доставке. В "
          "последний день часть заказов остаётся ожидающей доставки.</p>";
  text->setHtml(html);
  chart->update();
}
