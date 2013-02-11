TEMPLATE = app
TARGET   = tst_bench_events
QT       = core testlib
SOURCES  = $$PWD/../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt5/events/main.cpp
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
DESTDIR  = ../../../bin

include(../common.pri)
