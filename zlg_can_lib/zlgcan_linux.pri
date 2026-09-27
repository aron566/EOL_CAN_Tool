# 周立功 Linux 驱动库 (x86_64)
#
# 三个 ZCAN 库 (libusbcan-4e.so / libusbcan-8e.so / libusbcanfd800u.so) 导出同名 ZCAN_* 符号，
# 不能同时静态链接；驱动层 (can_driver_zlg.cpp) 按设备类型 dlopen(RTLD_LOCAL) + dlsym 动态加载。
# 因此这里只添加头文件路径、不做静态链接，构建后把 .so 部署到可执行文件输出目录。
#
# aarch64 合集里没有这三款设备的新 ZCAN 库，本 pri 仅在 x86_64 生效。

unix:!macx {
    contains(QMAKE_HOST.arch, x86_64) {
        ZLG_LINUX_DIR = $$PWD/zlgcan_linux_x86_64

        INCLUDEPATH += $$ZLG_LINUX_DIR/include
        DEPENDPATH += $$ZLG_LINUX_DIR/include

        # C++ 代码据此宏判断周立功 Linux 驱动是否可用
        DEFINES += ZLG_CAN_LINUX_SUPPORT

        # 运行时从程序目录 dlopen: 可执行文件带 $ORIGIN rpath
        # (QMAKE_RPATHDIR 会正确处理 $ORIGIN 的转义)
        QMAKE_RPATHDIR += $ORIGIN

        # 构建后把三个 ZCAN .so 复制到 DESTDIR (与可执行文件同目录)
        ZLG_LINUX_SO_FILES = libusbcan-4e.so libusbcan-8e.so libusbcanfd800u.so
        for(zlg_so, ZLG_LINUX_SO_FILES) {
            QMAKE_POST_LINK += $$quote(cp -f $$ZLG_LINUX_DIR/$$zlg_so $$DESTDIR/$$zlg_so) $$escape_expand(\\n\\t)
        }
    }
}
