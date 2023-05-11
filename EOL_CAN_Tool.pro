QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# 周立功can驱动库
include(zlg_can_lib/zlg_can_lib.pri)

# ini配置文件解析驱动库
include(ini_parse/ini_parse.pri)

# EOL协议
include(eol_protocol/eol_protocol.pri)

# 实用工具
include(utilities/utilities.pri)

# CAN驱动
include(can_driver/can_driver.pri)

SOURCES += \
    eol_angle_calibration_window/eol_angle_calibration_window.cpp \
    eol_calibration_window/eol_calibration_window.cpp \
    eol_sub_window/eol_sub_window.cpp \
    eol_window/eol_window.cpp \
    main.cpp \
    main_window/mainwindow.cpp \
    more_window/more_window.cpp

HEADERS += \
  eol_angle_calibration_window/eol_angle_calibration_window.h \
  eol_calibration_window/eol_calibration_window.h \
  eol_sub_window/eol_sub_window.h \
  eol_window/eol_window.h \
  main_window/mainwindow.h \
  more_window/more_window.h

FORMS += \
    eol_angle_calibration_window/eol_angle_calibration_window.ui \
    eol_calibration_window/eol_calibration_window.ui \
    eol_sub_window/eol_sub_window.ui \
    eol_window/eol_window.ui \
    main_window/mainwindow.ui \
    more_window/more_window.ui

# 包含路径
INCLUDEPATH += \
    main_window \
    more_window \
    eol_window \
    eol_sub_window \
    eol_calibration_window

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource/EOL_CAN_Tool.qrc \
    resource/qdarkstyle/dark/style.qrc
