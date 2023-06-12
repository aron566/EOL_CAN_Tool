FORMS += \
    $$PWD/serial_port_plotter.ui \
    $$PWD/serial_port_plotter_helpwindow.ui

HEADERS += \
    $$PWD/serial_port_plotter.hpp \
    $$PWD/serial_port_plotter_helpwindow.hpp \
    $$PWD/qcustomplot/qcustomplot.h

SOURCES += \
    $$PWD/serial_port_plotter.cpp \
    $$PWD/serial_port_plotter_helpwindow.cpp \
    $$PWD/qcustomplot/qcustomplot.cpp

INCLUDEPATH += $$PWD \
               $$PWD/qcustomplot
