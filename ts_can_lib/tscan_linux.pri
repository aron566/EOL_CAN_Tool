# 同星 (TOSUN) Linux 驱动库 (x86_64)
#
# 官方提供 libTSCANApiOnLinux.so (x86_64), TSCANLINApi.cpp 本来就用 QLibrary
# 动态加载 (Windows 下加载 ts_can_x64/libTSCAN.dll), Linux 下只需换个文件名，
# 无需像周立功那样按设备 dlopen 多库分发。
#
# 因此这里只做三件事:
#  1. 把 TSCANLINApi.h/.cpp 加入 Linux 构建 (头文件改用 Linux 兼容头，见下)
#  2. 定义 TSCAN_CAN_LINUX_SUPPORT, 供 mainwindow 按品牌 index 映射
#  3. 构建后把两个 .so 复制到 DESTDIR (与可执行文件同目录) 并设 $ORIGIN rpath
#
# 只有 x86_64 (官方无 aarch64 版本)。

unix:!macx {
    contains(QMAKE_HOST.arch, x86_64) {
        TSCAN_LINUX_DIR = $$PWD/tscan_linux_x86_64

        # TSCANLINApi.h/.cpp 参与 Linux 编译
        HEADERS += $$PWD/TSCANLINApi.h
        SOURCES += $$PWD/TSCANLINApi.cpp

        # TSCANLINApi.h 自身所在目录 + Linux 兼容头目录
        INCLUDEPATH += $$PWD
        INCLUDEPATH += $$TSCAN_LINUX_DIR/include
        DEPENDPATH += $$TSCAN_LINUX_DIR/include

        # C++ 代码据此宏判断同星 Linux 驱动是否可用
        DEFINES += TSCAN_CAN_LINUX_SUPPORT

        # 运行时从程序目录 QLibrary 加载: 可执行文件带 $ORIGIN rpath
        # (QMAKE_RPATHDIR 会正确处理 $ORIGIN 的转义)
        QMAKE_RPATHDIR += $ORIGIN

        # 构建后把两个 .so 复制到 DESTDIR (与可执行文件同目录)
        TSCAN_LINUX_SO_FILES = libTSCANApiOnLinux.so libTSH.so
        for(ts_so, TSCAN_LINUX_SO_FILES) {
            QMAKE_POST_LINK += $$quote(cp -f $$TSCAN_LINUX_DIR/lib/$$ts_so $$DESTDIR/$$ts_so) $$escape_expand(\\n\\t)
        }
    }
}
