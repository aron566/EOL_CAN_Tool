
contains(QMAKE_HOST.arch, x86_64) {
#    message("64-bit")
    # 64位编译器链接的库文件
    HEADERS += \
      $$PWD/zlgcan_x64/canframe.h \
      $$PWD/zlgcan_x64/config.h \
      $$PWD/zlgcan_x64/typedef.h \
      $$PWD/zlgcan_x64/zlgcan.h
    INCLUDEPATH += $$PWD/zlgcan_x64
    DEPENDPATH += $$PWD/zlgcan_x64
    LIBS += -L$$PWD/zlgcan_x64 -lzlgcan
} else {
#    message("32-bit")
    # 32位编译器链接的库文件
    HEADERS += \
      $$PWD/zlgcan_x86/canframe.h \
      $$PWD/zlgcan_x86/config.h \
      $$PWD/zlgcan_x86/typedef.h \
      $$PWD/zlgcan_x86/zlgcan.h
    INCLUDEPATH += $$PWD/zlgcan_x86
    DEPENDPATH += $$PWD/zlgcan_x86
    LIBS += -L$$PWD/zlgcan_x86 -lzlgcan
}
