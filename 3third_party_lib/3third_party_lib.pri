HEADERS += \
    $$PWD/hv/include/hv/*.h

INCLUDEPATH += $$PWD \
    $$PWD/hv/include

# binlog/BLF 仅 Windows (仓库内只有 Windows DLL/.lib, 无 Linux 版;
# 且 blf/Include 目录大小写在 Linux 下敏感, win32 分支保持原样不动)
win32 {
    HEADERS += \
        $$PWD/blf/include/*.h

    INCLUDEPATH += \
        $$PWD/blf/include
}

# libhv库文件 (MSVC编译, v1.3.4)
win32 {
contains(QMAKE_HOST.arch, x86_64) {
    # 64位编译器链接的库文件
    DEPENDPATH += $$PWD/hv/bin
    LIBS += -L$$PWD/hv/bin -lhv -lws2_32
} else {
    # 32位编译器链接的库文件
    DEPENDPATH += $$PWD/hv/bin
    LIBS += -L$$PWD/hv/bin -lhv -lws2_32
}
}
# Linux: libhv 需从源码自行编译安装 (cmake && make && sudo make install),
# 仓库内无预编译 .so, 这里只链接系统路径下的 libhv.so (libhv 内部使用 pthread)
unix:!macx: LIBS += -lhv -lpthread

# can binlog库文件 (仅 Windows)
win32 {
contains(QMAKE_HOST.arch, x86_64) {
    # 64位编译器链接的库文件
    DEPENDPATH += $$PWD/blf/LIB/x64_Release
    LIBS += -L$$PWD/blf/LIB/x64_Release -lbinlog
} else {
    # 32位编译器链接的库文件

}
}
