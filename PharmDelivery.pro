QT = core widgets

CONFIG += c++17 cmdline
INCLUDEPATH += $$PWD $$PWD/back

# Можно запретить сборку при использовании устаревших интерфейсов.
# Для этого раскомментируйте следующую строку.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # отключает интерфейсы, устаревшие до Qt 6.0.0

SOURCES += \
        customer.cpp \
        data.cpp \
        main.cpp \
        medicine.cpp \
        order.cpp \
        request.cpp \
        statistics.cpp \
        store.cpp \
        data_generator.cpp \
        internal/model_support.cpp \
        internal/supply_manager.cpp \
        internal/simulation_engine.cpp \
        internal/simulation_cli.cpp

# Стандартные правила установки приложения.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    back/customer.h \
    back/data.h \
    back/medicine.h \
    back/order.h \
    back/request.h \
    back/statistics.h \
    back/store.h \
    back/data_generator.h \
    internal/model_support.h \
    internal/supply_manager.h \
    back/simulation.h \
    internal/simulation_engine.h \
    internal/simulation_cli.h
