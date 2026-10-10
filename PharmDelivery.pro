QT += core gui widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
        back/customer.cpp \
        back/data.cpp \
        back/medicine.cpp \
        back/order.cpp \
        back/request.cpp \
        back/statistics.cpp \
        back/store.cpp \
        front/courierstab.cpp \
        front/logtab.cpp \
        front/orderstab.cpp \
        front/requeststab.cpp \
        front/stocktab.cpp \
        front/widgets/loadring.cpp \
        front/widgets/stockbar.cpp \
        main.cpp \
        mainwindow.cpp \

HEADERS += \
    back/customer.h \
    back/data.h \
    back/medicine.h \
    back/order.h \
    back/request.h \
    back/statistics.h \
    back/store.h \
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
