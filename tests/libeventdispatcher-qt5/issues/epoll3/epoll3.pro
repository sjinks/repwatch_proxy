QT       = core network testlib
TEMPLATE = app
TARGET   = tst_epoll3
SOURCES += $$PWD/../../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt5/issues/epoll3/tst_epoll3.cpp
DESTDIR  = ../../../../bin

include(../../common.pri)

win32: LIBS += -lws2_32
