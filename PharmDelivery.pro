QT += core gui widgets

CONFIG += c++17

INCLUDEPATH += $$PWD $$PWD/back

# You can make your code fail to compile if it uses deprecated APIs.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
        order.cpp \
        statistics.cpp \
        store.cpp \
        data_generator.cpp \
        internal/model_support.cpp \
        internal/supply_manager.cpp \
        internal/simulation_engine.cpp \
        front/courierstab.cpp \
        front/logtab.cpp \
        front/orderstab.cpp \
        front/requeststab.cpp \
        front/stocktab.cpp \
        front/widgets/loadring.cpp \
        front/widgets/stockbar.cpp \
        main.cpp \
        mainwindow.cpp

HEADERS += \
    back/customer.h \
    back/data.h \
    back/data_generator.h \
    back/medicine.h \
    back/order.h \
    back/request.h \
    back/statistics.h \
    back/store.h \
    back/simulation.h \
    internal/model_support.h \
    internal/supply_manager.h \
    internal/simulation_engine.h \
    front/courierstab.h \
    front/logtab.h \
    front/orderstab.h \
    front/requeststab.h \
    front/stocktab.h \
    front/widgets/loadring.h \
    front/widgets/stockbar.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    front/ff.txt
