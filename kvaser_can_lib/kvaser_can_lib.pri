HEADERS += \
  $$PWD/INC/*.h \
  $$PWD/INC/extras/*.h

SOURCES += \
    # $$PWD/TSCANLINApi.cpp

INCLUDEPATH += $$PWD/INC \
               $$PWD/INC/extras

# Kvaser库文件
contains(QMAKE_HOST.arch, x86_64) {
    # 64位编译器链接的库文件
    DEPENDPATH += $$PWD/kvaser_can_x64
    LIBS += -L$$PWD/kvaser_can_x64 -lcanlib32 
} else {
    # 32位编译器链接的库文件
    DEPENDPATH += $$PWD/kvaser_can_x86
    LIBS += -L$$PWD/kvaser_can_x86 -lcanlib32 
}
