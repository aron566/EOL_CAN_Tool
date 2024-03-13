HEADERS += \
    $$PWD/hv/include/hv/*.h \
    $$PWD/blf/include/*.h

INCLUDEPATH += $$PWD \
    $$PWD/hv/include \
    $$PWD/blf/include

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

# can binlog库文件
contains(QMAKE_HOST.arch, x86_64) {
    # 64位编译器链接的库文件
    DEPENDPATH += $$PWD/blf/LIB/x64_Release
    LIBS += -L$$PWD/blf/LIB/x64_Release -lbinlog
} else {
    # 32位编译器链接的库文件

}
