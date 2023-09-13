HEADERS += \
  $$PWD/TSCANLINApi.h \
  $$PWD/TSCANDef.h

SOURCES += \
    $$PWD/TSCANLINApi.cpp

INCLUDEPATH += $$PWD

# 同星库文件
contains(QMAKE_HOST.arch, x86_64) {
    # 64位编译器链接的库文件
    DEPENDPATH += $$PWD/ts_can_x64
    LIBS += -L$$PWD/ts_can_x64 -lTSCAN \
            -L$$PWD/ts_can_x64 -lTSH
} else {
    # 32位编译器链接的库文件
    DEPENDPATH += $$PWD/ts_can_x86
    LIBS += -L$$PWD/ts_can_x86 -lTSCAN \
            -L$$PWD/ts_can_x86 -lTSH
}
