HEADERS += \
    $$PWD/hv/include/hv/*.h

INCLUDEPATH += $$PWD \
    $$PWD/hv/include

# libhv库文件
contains(QMAKE_HOST.arch, x86_64) {
    # 64位编译器链接的库文件
    DEPENDPATH += $$PWD/hv/bin
    LIBS += -L$$PWD/hv/bin -llibhv64 -lws2_32
} else {
    # 32位编译器链接的库文件
    DEPENDPATH += $$PWD/hv/bin
    LIBS += -L$$PWD/hv/bin -llibhv32 -lws2_32
}
