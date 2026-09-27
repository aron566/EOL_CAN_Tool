HEADERS += \
  $$PWD/auto_dynamic_mem.h \
  $$PWD/block_queue.h \
  $$PWD/circularqueue.h \
  $$PWD/line_highlighter.h \
  $$PWD/listen_data.h \
  $$PWD/msg_log_buffer.h \
  $$PWD/safe_queue.h \
  $$PWD/utility.h

SOURCES += \
  $$PWD/auto_dynamic_mem.cpp \
  $$PWD/block_queue.cpp \
  $$PWD/circularqueue.cpp \
  $$PWD/line_highlighter.cpp \
  $$PWD/listen_data.cpp \
  $$PWD/msg_log_buffer.cpp \
  $$PWD/safe_queue.cpp \
  $$PWD/utility.cpp

# blf2asc 仅 Windows: 依赖 <windows.h>/<tchar.h> 与 binlog.dll,
# 且全仓库无调用方 (死代码), Linux 下剔除
win32 {
    HEADERS += $$PWD/blf2asc.h
    SOURCES += $$PWD/blf2asc.cpp
}

INCLUDEPATH += $$PWD
