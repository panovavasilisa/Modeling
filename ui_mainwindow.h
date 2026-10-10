/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "front/stocktab.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QHBoxLayout *QMenuBar;
    QPushButton *startBtn;
    QPushButton *pauseBtn;
    QPushButton *stepBtn;
    QPushButton *resetBtn;
    QLabel *label_2;
    QLineEdit *lineEdit;
    QLabel *label;
    QSlider *horizontalSlider;
    QTabWidget *tabs;
    QWidget *paramsTab;
    QGroupBox *groupBox;
    QWidget *formLayoutWidget;
    QFormLayout *formLayout;
    QLabel *label_5;
    QSpinBox *daysSpin;
    QSpacerItem *verticalSpacer;
    QLabel *label_6;
    QSpinBox *couriersSpin;
    QSpacerItem *verticalSpacer_2;
    QLabel *label_7;
    QSpinBox *medicinesSpin;
    QSpacerItem *verticalSpacer_3;
    QLabel *label_8;
    QDoubleSpinBox *capitalSpin;
    QGroupBox *groupBox_2;
    QWidget *formLayoutWidget_2;
    QFormLayout *formLayout_2;
    QLabel *label_9;
    QDoubleSpinBox *markupSpin;
    QSpacerItem *verticalSpacer_8;
    QLabel *label_10;
    QDoubleSpinBox *cardDiscountSpin;
    QSpacerItem *verticalSpacer_9;
    QLabel *label_11;
    QDoubleSpinBox *bigOrderDiscountSpin;
    QSpacerItem *verticalSpacer_10;
    QLabel *label_12;
    QDoubleSpinBox *regularDiscountSpin;
    QSpacerItem *verticalSpacer_11;
    QLabel *label_21;
    QDoubleSpinBox *maxDiscountSpin;
    QGroupBox *groupBox_3;
    QWidget *formLayoutWidget_3;
    QFormLayout *formLayout_3;
    QLabel *label_13;
    QSpinBox *thresholdSpin;
    QSpacerItem *verticalSpacer_5;
    QLabel *label_14;
    QSpinBox *expiryDaysSpin;
    QGroupBox *groupBox_4;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label_17;
    QSpinBox *ordersPerDaySpin;
    QSpacerItem *verticalSpacer_6;
    QCheckBox *dependencyCheck;
    QSpacerItem *verticalSpacer_7;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_19;
    QSpinBox *discountedProbSpin;
    QGroupBox *groupBox_5;
    QWidget *horizontalLayoutWidget_3;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *addBtn;
    QPushButton *generateBtn;
    QPushButton *deleteBtn;
    QTableView *tableView;
    QPushButton *saveBtn;
    StockTab *stockTab;
    QWidget *ordersTab;
    QWidget *couriersTab;
    QWidget *requestsTab;
    QWidget *resultsTab;
    QWidget *logTab;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1277, 641);
        gridLayoutWidget = new QWidget(MainWindow);
        gridLayoutWidget->setObjectName("gridLayoutWidget");
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setObjectName("gridLayout");
        QMenuBar = new QHBoxLayout();
        QMenuBar->setObjectName("QMenuBar");
        startBtn = new QPushButton(gridLayoutWidget);
        startBtn->setObjectName("startBtn");

        QMenuBar->addWidget(startBtn);

        pauseBtn = new QPushButton(gridLayoutWidget);
        pauseBtn->setObjectName("pauseBtn");

        QMenuBar->addWidget(pauseBtn);

        stepBtn = new QPushButton(gridLayoutWidget);
        stepBtn->setObjectName("stepBtn");

        QMenuBar->addWidget(stepBtn);

        resetBtn = new QPushButton(gridLayoutWidget);
        resetBtn->setObjectName("resetBtn");

        QMenuBar->addWidget(resetBtn);

        label_2 = new QLabel(gridLayoutWidget);
        label_2->setObjectName("label_2");

        QMenuBar->addWidget(label_2);

        lineEdit = new QLineEdit(gridLayoutWidget);
        lineEdit->setObjectName("lineEdit");

        QMenuBar->addWidget(lineEdit);

        label = new QLabel(gridLayoutWidget);
        label->setObjectName("label");

        QMenuBar->addWidget(label);

        horizontalSlider = new QSlider(gridLayoutWidget);
        horizontalSlider->setObjectName("horizontalSlider");
        horizontalSlider->setOrientation(Qt::Orientation::Horizontal);

        QMenuBar->addWidget(horizontalSlider);


        gridLayout->addLayout(QMenuBar, 0, 0, 1, 1);

        tabs = new QTabWidget(gridLayoutWidget);
        tabs->setObjectName("tabs");
        tabs->setAcceptDrops(false);
        paramsTab = new QWidget();
        paramsTab->setObjectName("paramsTab");
        groupBox = new QGroupBox(paramsTab);
        groupBox->setObjectName("groupBox");
        groupBox->setGeometry(QRect(20, 30, 231, 161));
        formLayoutWidget = new QWidget(groupBox);
        formLayoutWidget->setObjectName("formLayoutWidget");
        formLayoutWidget->setGeometry(QRect(10, 19, 211, 131));
        formLayout = new QFormLayout(formLayoutWidget);
        formLayout->setObjectName("formLayout");
        formLayout->setContentsMargins(0, 0, 0, 0);
        label_5 = new QLabel(formLayoutWidget);
        label_5->setObjectName("label_5");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, label_5);

        daysSpin = new QSpinBox(formLayoutWidget);
        daysSpin->setObjectName("daysSpin");

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, daysSpin);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        formLayout->setItem(1, QFormLayout::ItemRole::FieldRole, verticalSpacer);

        label_6 = new QLabel(formLayoutWidget);
        label_6->setObjectName("label_6");

        formLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, label_6);

        couriersSpin = new QSpinBox(formLayoutWidget);
        couriersSpin->setObjectName("couriersSpin");

        formLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, couriersSpin);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        formLayout->setItem(3, QFormLayout::ItemRole::FieldRole, verticalSpacer_2);

        label_7 = new QLabel(formLayoutWidget);
        label_7->setObjectName("label_7");

        formLayout->setWidget(4, QFormLayout::ItemRole::LabelRole, label_7);

        medicinesSpin = new QSpinBox(formLayoutWidget);
        medicinesSpin->setObjectName("medicinesSpin");

        formLayout->setWidget(4, QFormLayout::ItemRole::FieldRole, medicinesSpin);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        formLayout->setItem(5, QFormLayout::ItemRole::FieldRole, verticalSpacer_3);

        label_8 = new QLabel(formLayoutWidget);
        label_8->setObjectName("label_8");

        formLayout->setWidget(6, QFormLayout::ItemRole::LabelRole, label_8);

        capitalSpin = new QDoubleSpinBox(formLayoutWidget);
        capitalSpin->setObjectName("capitalSpin");

        formLayout->setWidget(6, QFormLayout::ItemRole::FieldRole, capitalSpin);

        groupBox_2 = new QGroupBox(paramsTab);
        groupBox_2->setObjectName("groupBox_2");
        groupBox_2->setGeometry(QRect(320, 240, 261, 181));
        formLayoutWidget_2 = new QWidget(groupBox_2);
        formLayoutWidget_2->setObjectName("formLayoutWidget_2");
        formLayoutWidget_2->setGeometry(QRect(10, 20, 251, 151));
        formLayout_2 = new QFormLayout(formLayoutWidget_2);
        formLayout_2->setObjectName("formLayout_2");
        formLayout_2->setContentsMargins(0, 0, 0, 0);
        label_9 = new QLabel(formLayoutWidget_2);
        label_9->setObjectName("label_9");

        formLayout_2->setWidget(0, QFormLayout::ItemRole::LabelRole, label_9);

        markupSpin = new QDoubleSpinBox(formLayoutWidget_2);
        markupSpin->setObjectName("markupSpin");

        formLayout_2->setWidget(0, QFormLayout::ItemRole::FieldRole, markupSpin);

        verticalSpacer_8 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        formLayout_2->setItem(1, QFormLayout::ItemRole::FieldRole, verticalSpacer_8);

        label_10 = new QLabel(formLayoutWidget_2);
        label_10->setObjectName("label_10");

        formLayout_2->setWidget(2, QFormLayout::ItemRole::LabelRole, label_10);

        cardDiscountSpin = new QDoubleSpinBox(formLayoutWidget_2);
        cardDiscountSpin->setObjectName("cardDiscountSpin");

        formLayout_2->setWidget(2, QFormLayout::ItemRole::FieldRole, cardDiscountSpin);

        verticalSpacer_9 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        formLayout_2->setItem(3, QFormLayout::ItemRole::FieldRole, verticalSpacer_9);

        label_11 = new QLabel(formLayoutWidget_2);
        label_11->setObjectName("label_11");

        formLayout_2->setWidget(4, QFormLayout::ItemRole::LabelRole, label_11);

        bigOrderDiscountSpin = new QDoubleSpinBox(formLayoutWidget_2);
        bigOrderDiscountSpin->setObjectName("bigOrderDiscountSpin");

        formLayout_2->setWidget(4, QFormLayout::ItemRole::FieldRole, bigOrderDiscountSpin);

        verticalSpacer_10 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        formLayout_2->setItem(5, QFormLayout::ItemRole::FieldRole, verticalSpacer_10);

        label_12 = new QLabel(formLayoutWidget_2);
        label_12->setObjectName("label_12");

        formLayout_2->setWidget(6, QFormLayout::ItemRole::LabelRole, label_12);

        regularDiscountSpin = new QDoubleSpinBox(formLayoutWidget_2);
        regularDiscountSpin->setObjectName("regularDiscountSpin");

        formLayout_2->setWidget(6, QFormLayout::ItemRole::FieldRole, regularDiscountSpin);

        verticalSpacer_11 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        formLayout_2->setItem(7, QFormLayout::ItemRole::FieldRole, verticalSpacer_11);

        label_21 = new QLabel(formLayoutWidget_2);
        label_21->setObjectName("label_21");

        formLayout_2->setWidget(8, QFormLayout::ItemRole::LabelRole, label_21);

        maxDiscountSpin = new QDoubleSpinBox(formLayoutWidget_2);
        maxDiscountSpin->setObjectName("maxDiscountSpin");

        formLayout_2->setWidget(8, QFormLayout::ItemRole::FieldRole, maxDiscountSpin);

        groupBox_3 = new QGroupBox(paramsTab);
        groupBox_3->setObjectName("groupBox_3");
        groupBox_3->setGeometry(QRect(320, 30, 261, 81));
        formLayoutWidget_3 = new QWidget(groupBox_3);
        formLayoutWidget_3->setObjectName("formLayoutWidget_3");
        formLayoutWidget_3->setGeometry(QRect(9, 20, 251, 61));
        formLayout_3 = new QFormLayout(formLayoutWidget_3);
        formLayout_3->setObjectName("formLayout_3");
        formLayout_3->setContentsMargins(0, 0, 0, 0);
        label_13 = new QLabel(formLayoutWidget_3);
        label_13->setObjectName("label_13");

        formLayout_3->setWidget(0, QFormLayout::ItemRole::LabelRole, label_13);

        thresholdSpin = new QSpinBox(formLayoutWidget_3);
        thresholdSpin->setObjectName("thresholdSpin");

        formLayout_3->setWidget(0, QFormLayout::ItemRole::FieldRole, thresholdSpin);

        verticalSpacer_5 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        formLayout_3->setItem(1, QFormLayout::ItemRole::FieldRole, verticalSpacer_5);

        label_14 = new QLabel(formLayoutWidget_3);
        label_14->setObjectName("label_14");

        formLayout_3->setWidget(2, QFormLayout::ItemRole::LabelRole, label_14);

        expiryDaysSpin = new QSpinBox(formLayoutWidget_3);
        expiryDaysSpin->setObjectName("expiryDaysSpin");

        formLayout_3->setWidget(2, QFormLayout::ItemRole::FieldRole, expiryDaysSpin);

        groupBox_4 = new QGroupBox(paramsTab);
        groupBox_4->setObjectName("groupBox_4");
        groupBox_4->setGeometry(QRect(20, 240, 231, 131));
        verticalLayoutWidget = new QWidget(groupBox_4);
        verticalLayoutWidget->setObjectName("verticalLayoutWidget");
        verticalLayoutWidget->setGeometry(QRect(10, 20, 211, 101));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        label_17 = new QLabel(verticalLayoutWidget);
        label_17->setObjectName("label_17");

        horizontalLayout->addWidget(label_17);

        ordersPerDaySpin = new QSpinBox(verticalLayoutWidget);
        ordersPerDaySpin->setObjectName("ordersPerDaySpin");

        horizontalLayout->addWidget(ordersPerDaySpin);


        verticalLayout->addLayout(horizontalLayout);

        verticalSpacer_6 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer_6);

        dependencyCheck = new QCheckBox(verticalLayoutWidget);
        dependencyCheck->setObjectName("dependencyCheck");

        verticalLayout->addWidget(dependencyCheck);

        verticalSpacer_7 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer_7);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        label_19 = new QLabel(verticalLayoutWidget);
        label_19->setObjectName("label_19");

        horizontalLayout_2->addWidget(label_19);

        discountedProbSpin = new QSpinBox(verticalLayoutWidget);
        discountedProbSpin->setObjectName("discountedProbSpin");

        horizontalLayout_2->addWidget(discountedProbSpin);


        verticalLayout->addLayout(horizontalLayout_2);

        groupBox_5 = new QGroupBox(paramsTab);
        groupBox_5->setObjectName("groupBox_5");
        groupBox_5->setGeometry(QRect(690, 30, 541, 301));
        horizontalLayoutWidget_3 = new QWidget(groupBox_5);
        horizontalLayoutWidget_3->setObjectName("horizontalLayoutWidget_3");
        horizontalLayoutWidget_3->setGeometry(QRect(20, 30, 271, 31));
        horizontalLayout_3 = new QHBoxLayout(horizontalLayoutWidget_3);
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        addBtn = new QPushButton(horizontalLayoutWidget_3);
        addBtn->setObjectName("addBtn");

        horizontalLayout_3->addWidget(addBtn);

        generateBtn = new QPushButton(horizontalLayoutWidget_3);
        generateBtn->setObjectName("generateBtn");

        horizontalLayout_3->addWidget(generateBtn);

        deleteBtn = new QPushButton(horizontalLayoutWidget_3);
        deleteBtn->setObjectName("deleteBtn");

        horizontalLayout_3->addWidget(deleteBtn);

        tableView = new QTableView(groupBox_5);
        tableView->setObjectName("tableView");
        tableView->setGeometry(QRect(20, 71, 501, 181));
        saveBtn = new QPushButton(groupBox_5);
        saveBtn->setObjectName("saveBtn");
        saveBtn->setGeometry(QRect(20, 260, 91, 31));
        tabs->addTab(paramsTab, QString());
        stockTab = new StockTab();
        stockTab->setObjectName("stockTab");
        tabs->addTab(stockTab, QString());
        ordersTab = new QWidget();
        ordersTab->setObjectName("ordersTab");
        tabs->addTab(ordersTab, QString());
        couriersTab = new QWidget();
        couriersTab->setObjectName("couriersTab");
        tabs->addTab(couriersTab, QString());
        requestsTab = new QWidget();
        requestsTab->setObjectName("requestsTab");
        tabs->addTab(requestsTab, QString());
        resultsTab = new QWidget();
        resultsTab->setObjectName("resultsTab");
        tabs->addTab(resultsTab, QString());
        logTab = new QWidget();
        logTab->setObjectName("logTab");
        tabs->addTab(logTab, QString());

        gridLayout->addWidget(tabs, 1, 0, 1, 1);

        MainWindow->setCentralWidget(gridLayoutWidget);

        retranslateUi(MainWindow);

        tabs->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "\320\234\320\276\320\264\320\265\320\273\320\270\321\200\320\276\320\262\320\260\320\275\320\270\320\265 \321\201\320\273\321\203\320\266\320\261\321\213 \320\264\320\276\321\201\321\202\320\260\320\262\320\272\320\270 \320\273\320\265\320\272\320\260\321\200\321\201\321\202\320\262", nullptr));
        startBtn->setText(QCoreApplication::translate("MainWindow", "\320\241\321\202\320\260\321\200\321\202", nullptr));
        pauseBtn->setText(QCoreApplication::translate("MainWindow", "\320\237\320\260\321\203\320\267\320\260", nullptr));
        stepBtn->setText(QCoreApplication::translate("MainWindow", "\320\250\320\260\320\263", nullptr));
        resetBtn->setText(QCoreApplication::translate("MainWindow", "\320\241\320\261\321\200\320\276\321\201", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "\320\224\320\265\320\275\321\214:", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "\320\241\320\272\320\276\321\200\320\276\321\201\321\202\321\214", nullptr));
        groupBox->setTitle(QCoreApplication::translate("MainWindow", "\320\236\320\261\321\211\320\270\320\265", nullptr));
        label_5->setText(QCoreApplication::translate("MainWindow", "\320\224\320\275\320\265\320\271 (N)", nullptr));
        label_6->setText(QCoreApplication::translate("MainWindow", "\320\232\321\203\321\200\321\214\320\265\321\200\320\276\320\262 (M)", nullptr));
        label_7->setText(QCoreApplication::translate("MainWindow", "\320\233\320\265\320\272\320\260\321\200\321\201\321\202\320\262 (K)", nullptr));
        label_8->setText(QCoreApplication::translate("MainWindow", "\320\232\320\260\320\277\320\270\321\202\320\260\320\273", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("MainWindow", "\320\244\320\270\320\275\320\260\320\275\321\201\321\213", nullptr));
        label_9->setText(QCoreApplication::translate("MainWindow", "\320\240\320\276\320\267\320\275\320\270\321\207\320\275\320\260\321\217 \320\275\320\260\321\206\320\265\320\275\320\272\320\260, %", nullptr));
        label_10->setText(QCoreApplication::translate("MainWindow", "\320\241\320\272\320\270\320\264\320\272\320\260 \320\277\320\276 \320\272\320\260\321\200\321\202\320\265, %", nullptr));
        label_11->setText(QCoreApplication::translate("MainWindow", "\320\241\320\272\320\270\320\264\320\272\320\260 \320\277\321\200\320\270 >1000\321\200, %", nullptr));
        label_12->setText(QCoreApplication::translate("MainWindow", "\320\241\320\272\320\270\320\264\320\272\320\260 \320\277\320\276\321\201\321\202\320\276\321\217\320\275\320\275\321\213\320\274, %", nullptr));
        label_21->setText(QCoreApplication::translate("MainWindow", "\320\234\320\260\320\272\321\201\320\270\320\274\320\260\320\273\321\214\320\275\320\260\321\217 \321\201\320\272\320\270\320\264\320\272\320\260, %", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("MainWindow", "\320\241\320\272\320\273\320\260\320\264", nullptr));
        label_13->setText(QCoreApplication::translate("MainWindow", "\320\237\320\276\321\200\320\276\320\263 \320\267\320\260\321\217\320\262\320\272\320\270", nullptr));
        label_14->setText(QCoreApplication::translate("MainWindow", "\320\241\321\200\320\276\320\272 \321\203\321\206\320\265\320\275\320\272\320\270", nullptr));
        groupBox_4->setTitle(QCoreApplication::translate("MainWindow", "\320\237\320\276\321\202\320\276\320\272 \320\267\320\260\320\272\320\260\320\267\320\276\320\262", nullptr));
        label_17->setText(QCoreApplication::translate("MainWindow", "\320\227\320\260\320\272\320\260\320\267\320\276\320\262 \320\262 \320\264\320\265\320\275\321\214                ", nullptr));
        dependencyCheck->setText(QCoreApplication::translate("MainWindow", "\320\227\320\260\320\262\320\270\321\201\320\270\320\274\320\276\321\201\321\202\321\214 \320\276\321\202 \320\275\320\260\321\206\320\265\320\275\320\272\320\270", nullptr));
        label_19->setText(QCoreApplication::translate("MainWindow", "\320\222\320\265\321\200\320\276\321\217\321\202\320\275\320\276\321\201\321\202\321\214 \321\203\321\206\320\265\320\275\321\221\320\275\320\275\321\213\321\205", nullptr));
        groupBox_5->setTitle(QCoreApplication::translate("MainWindow", "\320\235\320\260\321\207\320\260\320\273\321\214\320\275\321\213\320\271 \320\275\320\260\320\261\320\276\321\200 \320\273\320\265\320\272\320\260\321\200\321\201\321\202\320\262", nullptr));
        addBtn->setText(QCoreApplication::translate("MainWindow", "\320\224\320\276\320\261\320\260\320\262\320\270\321\202\321\214", nullptr));
        generateBtn->setText(QCoreApplication::translate("MainWindow", "\320\241\320\263\320\265\320\275\320\265\321\200\320\270\321\200\320\276\320\262\320\260\321\202\321\214", nullptr));
        deleteBtn->setText(QCoreApplication::translate("MainWindow", "\320\243\320\264\320\260\320\273\320\270\321\202\321\214", nullptr));
        saveBtn->setText(QCoreApplication::translate("MainWindow", "\320\241\320\276\321\205\321\200\320\260\320\275\320\270\321\202\321\214", nullptr));
        tabs->setTabText(tabs->indexOf(paramsTab), QCoreApplication::translate("MainWindow", "\320\237\320\260\321\200\320\260\320\274\320\265\321\202\321\200\321\213", nullptr));
        tabs->setTabText(tabs->indexOf(stockTab), QCoreApplication::translate("MainWindow", "\320\241\320\272\320\273\320\260\320\264", nullptr));
        tabs->setTabText(tabs->indexOf(ordersTab), QCoreApplication::translate("MainWindow", "\320\227\320\260\320\272\320\260\320\267\321\213", nullptr));
        tabs->setTabText(tabs->indexOf(couriersTab), QCoreApplication::translate("MainWindow", "\320\232\321\203\321\200\321\214\320\265\321\200\321\213", nullptr));
        tabs->setTabText(tabs->indexOf(requestsTab), QCoreApplication::translate("MainWindow", "\320\227\320\260\321\217\320\262\320\272\320\270", nullptr));
        tabs->setTabText(tabs->indexOf(resultsTab), QCoreApplication::translate("MainWindow", "\320\240\320\265\320\267\321\203\320\273\321\214\321\202\320\260\321\202\321\213", nullptr));
        tabs->setTabText(tabs->indexOf(logTab), QCoreApplication::translate("MainWindow", "\320\226\321\203\321\200\320\275\320\260\320\273", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
