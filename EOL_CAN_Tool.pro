QT       += core gui
QT       += multimedia
QT       += serialport
QT       += sql
QT       += charts
QT       += svg
QT       += xml
QT       += network
QT       += concurrent
# modbus / can driver
QT       += serialbus
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

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

# 设置目标生成路径
unix:!macx: DESTDIR = ../../bin
win32: DESTDIR = "$(USERPROFILE)\\Desktop\\EOL_CAN_Tool"

# 执行打包工具
QMAKE_PRE_LINK += echo start Build $$TARGET

# 拷贝libhv库文件
contains(QMAKE_HOST.arch, x86_64) {
    # 64位编译器链接的库文件
    copy_hvfiles.commands = powershell -Command "Copy-Item -Path $$PWD/3third_party_lib/hv/bin/libhv64.dll -Destination $$DESTDIR/libhv.dll -Force"
    QMAKE_EXTRA_TARGETS += copy_hvfiles
    PRE_TARGETDEPS += copy_hvfiles
    copy_cryptofiles.commands = powershell -Command "Copy-Item -Path $$PWD/3third_party_lib/hv/lib/libcrypto-1_1-x64.dll -Destination $$DESTDIR/ -Force"
    QMAKE_EXTRA_TARGETS += copy_cryptofiles
    PRE_TARGETDEPS += copy_cryptofiles
    copy_sslfiles.commands = powershell -Command "Copy-Item -Path $$PWD/3third_party_lib/hv/lib/libssl-1_1-x64.dll -Destination $$DESTDIR/ -Force"
    QMAKE_EXTRA_TARGETS += copy_sslfiles
    PRE_TARGETDEPS += copy_sslfiles
} else {
    # 32位编译器链接的库文件
    copy_hvfiles.commands = powershell -Command "Copy-Item -Path $$PWD/3third_party_lib/hv/bin/libhv32.dll -Destination $$DESTDIR/libhv.dll -Force"
    QMAKE_EXTRA_TARGETS += copy_hvfiles
    PRE_TARGETDEPS += copy_hvfiles
    copy_cryptofiles.commands = powershell -Command "Copy-Item -Path $$PWD/3third_party_lib/hv/lib/libcrypto-1_1.dll -Destination $$DESTDIR/ -Force"
    QMAKE_EXTRA_TARGETS += copy_cryptofiles
    PRE_TARGETDEPS += copy_cryptofiles
    copy_sslfiles.commands = powershell -Command "Copy-Item -Path $$PWD/3third_party_lib/hv/lib/libssl-1_1.dll -Destination $$DESTDIR/ -Force"
    QMAKE_EXTRA_TARGETS += copy_sslfiles
    PRE_TARGETDEPS += copy_sslfiles
}

win32: QMAKE_POST_LINK += $$DESTDIR/qtenvPackage.bat $$DESTDIR $${TARGET}.exe

# 周立功can驱动库
include(zlg_can_lib/zlg_can_lib.pri)

# 广成can驱动库
include(gc_can_lib/gc_can_lib.pri)

# 同星can驱动库
include(ts_can_lib/ts_can_lib.pri)

# kvasercan驱动库
include(kvaser_can_lib/kvaser_can_lib.pri)

# libhv网络驱动库
include(3third_party_lib\3third_party_lib.pri)

# ini配置文件解析驱动库
include(ini_parse/ini_parse.pri)

# EOL协议
include(eol_protocol/eol_protocol.pri)

# RTS协议
include(rts_protocol/rts_protocol.pri)

# 实用工具
include(utilities/utilities.pri)

# CAN驱动
include(can_driver/can_driver.pri)

# 网络驱动
include(network_driver/network_driver.pri)

# 工具集
## 图形工具
### 数据曲线
include(middleware/serial_port_plotter/serial_port_plotter.pri)

# 更新工具
include(middleware/Updater/QSimpleUpdater/QSimpleUpdater.pri)

SOURCES += \
    debug_window/debug_window.cpp \
    eol_angle_calibration_window/eol_angle_calibration_window.cpp \
    eol_calibration_window/eol_calibration_window.cpp \
    eol_rdm_debug_window/eol_rdm_debug_window.cpp \
    eol_sub_more_window/eol_sub_more_window.cpp \
    eol_sub_window/eol_sub_window.cpp \
    eol_window/eol_window.cpp \
    frame_diagnosis_window/frame_diagnosis.cpp \
    main.cpp \
    main_window/mainwindow.cpp \
    more_window/more_window.cpp \
    network_window/network_window.cpp \
    rts_ctrl_window/rts_ctrl_window.cpp \
    tool_window/tool_window.cpp \
    updater_window/updater_window.cpp

HEADERS += \
    debug_window/debug_window.h \
    eol_angle_calibration_window/eol_angle_calibration_window.h \
    eol_calibration_window/eol_calibration_window.h \
    eol_rdm_debug_window/eol_rdm_debug_window.h \
    eol_sub_more_window/eol_sub_more_window.h \
    eol_sub_window/eol_sub_window.h \
    eol_window/eol_window.h \
    frame_diagnosis_window/frame_diagnosis.h \
    main_window/mainwindow.h \
    more_window/more_window.h \
    network_window/network_window.h \
    rts_ctrl_window/rts_ctrl_window.h \
    tool_window/tool_window.h \
    updater_window/updater_window.h

FORMS += \
    debug_window/debug_window.ui \
    eol_angle_calibration_window/eol_angle_calibration_window.ui \
    eol_calibration_window/eol_calibration_window.ui \
    eol_rdm_debug_window/eol_rdm_debug_window.ui \
    eol_sub_more_window/eol_sub_more_window.ui \
    eol_sub_window/eol_sub_window.ui \
    eol_window/eol_window.ui \
    frame_diagnosis_window/frame_diagnosis.ui \
    main_window/mainwindow.ui \
    more_window/more_window.ui \
    network_window/network_window.ui \
    rts_ctrl_window/rts_ctrl_window.ui \
    tool_window/tool_window.ui \
    updater_window/updater_window.ui

# 包含路径
INCLUDEPATH += \
    main_window \
    more_window \
    eol_window \
    eol_sub_window \
    eol_calibration_window \
    updater_window

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource/EOL_CAN_Tool.qrc \
    resource/qdarkstyle/dark/style.qrc

RC_FILE = resource/EOL_CAN_Tool.rc
