HEADERS += \
  $$PWD/ecanvci.h

#DISTFILES += \
#  $$PWD/ECanVci64.lib

INCLUDEPATH += $$PWD

# 广成库文件
contains(QMAKE_TARGET.arch, x86_64) {
    # 64位平台链接的库文件
    contains(QMAKE_HOST.os, linux) {
        # Linux平台链接的库文件
#        LIBS += -L$$PWD -lECanVci64
    } else {
        # Windows平台链接的库文件
        LIBS += -L$$PWD -lECanVci64.lib
    }
} else {
    # 32位平台链接的库文件
    contains(QMAKE_HOST.os, linux) {
        # Linux平台链接的库文件
#        LIBS += -L$$PWD -lECanVci
    } else {
        # Windows平台链接的库文件
        LIBS += -L$$PWD -lECanVci.lib
    }
}
