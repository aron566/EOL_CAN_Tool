HEADERS += \
  $$PWD/ecanvci.h \
  $$PWD/ecanfdvci.h

INCLUDEPATH += $$PWD

# 广成库文件
contains(QMAKE_HOST.arch, x86_64) {
    # 64位编译器链接的库文件
    DEPENDPATH += $$PWD/GCANx64
    LIBS += -L$$PWD/GCANx64 -lECanVci64 \
            -L$$PWD/GCANx64 -lECANFDVCI64
} else {
    # 32位编译器链接的库文件
    DEPENDPATH += $$PWD/GCANx86
    LIBS += -L$$PWD/GCANx86 -lECanVci \
            -L$$PWD/GCANx86 -lECANFDVCI
}
