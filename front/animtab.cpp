#include "animtab.h"
#include <QPainter>
#include <cmath>
#include <QDebug>

AnimTab::AnimTab(QWidget* parent) : QWidget(parent), currentDay(0), totalStock(0), requestTimer(0) {
    imgWarehouse.load(":/images/warehouse.png");
    imgWarehouse = imgWarehouse.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if (imgWarehouse.isNull()) qDebug() << "ОШИБКА: Не найдена картинка склада!";

    imgHouse.load(":/images/house.png");
    imgHouse = imgHouse.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if (imgHouse.isNull()) qDebug() << "ОШИБКА: Не найдена картинка дома!";

    imgCourier.load(":/images/courier.png");
    imgCourier = imgCourier.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if (imgCourier.isNull()) qDebug() << "ОШИБКА: Не найдена картинка курьера!";

    imgPhone.load(":/images/phone.png");
    imgPhone = imgPhone.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if (imgPhone.isNull()) qDebug() << "ОШИБКА: Не найдена картинка телефона!";

    animTimer = new QTimer(this);
    connect(animTimer, &QTimer::timeout, this, &AnimTab::animate);
    animTimer->start(16);
    setMinimumSize(600, 400);
}

void AnimTab::updateState(int day, int stockCount, const std::vector<int>& loads, int newRequests) {
    currentDay = day;
    totalStock = stockCount;

    if (couriers.size() != loads.size()) {
        couriers.clear();
        for (size_t i = 0; i < loads.size(); ++i) {
            Courier c;
            c.x = 250;
            c.baseY = 150 + i * 50;
            c.y = c.baseY;
            c.speed = 3.0 + (i % 3);
            c.targetX = 250;
            c.state = Courier::Idle;
            c.deliveries = 0;
            couriers.push_back(c);
        }
    }

    for (size_t i = 0; i < couriers.size(); ++i) {
        couriers[i].deliveries = loads[i];
    }

    if (newRequests > 0) requestTimer = 120;
    update();
}

void AnimTab::animate() {
    bool needsRedraw = false;

    for (auto& c : couriers) {
        if (c.state == Courier::Idle && c.deliveries > 0) {
            c.state = Courier::GoingToCity;
            c.targetX = width() - 200;
        }

        if (c.state == Courier::GoingToCity) {
            c.x += c.speed;
            c.y = c.baseY + std::sin(c.x * 0.03) * 20;
            if (c.x >= c.targetX) {
                c.x = c.targetX;
                c.state = Courier::Returning;
                c.targetX = 250;
                c.deliveries = 0;
            }
            needsRedraw = true;
        } else if (c.state == Courier::Returning) {
            c.x -= c.speed;
            c.y = c.baseY + std::sin(c.x * 0.03) * 20;
            if (c.x <= c.targetX) {
                c.x = c.targetX;
                c.state = Courier::Idle;
            }
            needsRedraw = true;
        }
    }

    if (requestTimer > 0) { requestTimer--; needsRedraw = true; }
    if (needsRedraw) update();
}

void AnimTab::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.fillRect(rect(), QColor("#f0f4f8"));

    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.drawText(20, 30, QString("День: %1 | Остаток: %2 упак.").arg(currentDay).arg(totalStock));

    if (!imgWarehouse.isNull()) painter.drawPixmap(40, 150, 200, 200, imgWarehouse);
    else { painter.setBrush(QColor("#4a90e2")); painter.drawRect(40, 150, 100, 100); }

    for (int i = 0; i < 3; ++i) {
        if (!imgHouse.isNull()) painter.drawPixmap(width() - 200, 60 + i * 160, 150, 150, imgHouse);
        else { painter.setBrush(QColor("#7ed321")); painter.drawRect(width() - 150, 120 + i * 80, 80, 80); }
    }

    for (size_t i = 0; i < couriers.size(); ++i) {
        const auto& c = couriers[i];
        if (!imgCourier.isNull()) painter.drawPixmap(c.x - 25, c.y - 25, 50, 50, imgCourier);
        else {
            painter.setBrush(QColor("#f1c40f"));
            painter.drawEllipse(QPoint(c.x, c.y), 15, 15);
            painter.setPen(Qt::black);
            painter.drawText(c.x - 5, c.y + 5, QString::number(i + 1));
        }
    }

    if (requestTimer > 0) {
        painter.setOpacity((requestTimer % 20) / 20.0);
        if (!imgPhone.isNull()) painter.drawPixmap(110, 80, 60, 60, imgPhone);
        else { painter.setPen(QPen(Qt::red, 3)); painter.drawText(110, 80, "ЗАЯВКА!"); }
        painter.setOpacity(1.0);
    }
}

void AnimTab::resetAnimation() {
    for (auto& c : couriers) {
        c.x = 250;
        c.y = c.baseY;
        c.targetX = 250;
        c.state = Courier::Idle;
        c.deliveries = 0;
    }
    currentDay = 0;
    totalStock = 0;
    requestTimer = 0;
    update();
}

void AnimTab::pauseAnimation() {
    animTimer->stop();
}

void AnimTab::resumeAnimation() {
    if (!animTimer->isActive()) {
        animTimer->start(16);
    }
}
