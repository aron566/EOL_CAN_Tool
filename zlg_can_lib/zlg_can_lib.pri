HEADERS += \
  $$PWD/canframe.h \
  $$PWD/config.h \
  $$PWD/typedef.h \
  $$PWD/zlgcan.h

DISTFILES += \
  $$PWD/zlgcan.lib

INCLUDEPATH += $$PWD

# 周立功库文件
win32:CONFIG(release, debug|release): LIBS += -L$$PWD -lzlgcan
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD -lzlgcan

unix:!macx: LIBS += \
    -L$$PWD \
    -lzlgcan
