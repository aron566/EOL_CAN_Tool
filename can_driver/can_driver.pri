# 跨平台 CAN 驱动基类 (Windows/Linux 通用)
HEADERS += \
  $$PWD/can_driver_model.h \
  $$PWD/can_driver_sender.h

SOURCES += \
  $$PWD/can_driver_model.cpp \
  $$PWD/can_driver_sender.cpp

# 四个厂商驱动仅 Windows 编译 (仓库内只有 Windows DLL/.lib, 无 Linux 版 SDK)
win32 {
    HEADERS += \
      $$PWD/can_driver_gc.h \
      $$PWD/can_driver_kvaser.h \
      $$PWD/can_driver_ts.h \
      $$PWD/can_driver_zlg.h

    SOURCES += \
      $$PWD/can_driver_gc.cpp \
      $$PWD/can_driver_kvaser.cpp \
      $$PWD/can_driver_ts.cpp \
      $$PWD/can_driver_zlg.cpp
}

# SocketCAN 驱动: Linux 首版默认 CAN 通路 (依赖 Qt SerialBus 的 socketcan 插件)
unix:!macx {
    HEADERS += \
      $$PWD/can_driver_socketcan.h

    SOURCES += \
      $$PWD/can_driver_socketcan.cpp
}

# 周立功驱动: Linux(x86_64) 下通过 dlopen 动态加载官方 .so；aarch64 暂无可用库不编译
unix:!macx {
    contains(QMAKE_HOST.arch, x86_64) {
        HEADERS += \
          $$PWD/can_driver_zlg.h

        SOURCES += \
          $$PWD/can_driver_zlg.cpp
    }
}

INCLUDEPATH += $$PWD
