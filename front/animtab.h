#pragma once
#include <QWidget>
#include <QTimer>
#include <QPixmap>
#include <vector>

class AnimTab : public QWidget {
    Q_OBJECT
public:
    explicit AnimTab(QWidget* parent = nullptr);
    void updateState(int day, int stockCount, const std::vector<int>& loads, int newRequests);
    void resetAnimation();
    void pauseAnimation();
    void resumeAnimation();
    void setSpeedMultiplier(double mult);

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void animate();

private:
    double speedMultiplier;

    struct Courier {
        double x, y;
        double baseY;
        double targetX;
        double speed;
        enum State { Idle, GoingToCity, Returning } state;
        int deliveries;
    };

    QTimer* animTimer;
    int currentDay;
    int totalStock;
    int requestTimer;
    std::vector<Courier> couriers;

    QPixmap imgWarehouse;
    QPixmap imgHouse;
    QPixmap imgCourier;
    QPixmap imgPhone;
};
