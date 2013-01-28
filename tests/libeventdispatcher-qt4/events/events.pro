TEMPLATE = app
TARGET   = tst_bench_events
QT       = core testlib
SOURCES  = $$PWD/../../../libs/qt_eventdispatcher_libevent/tests-qt4/events/main.cpp
DESTDIR  = ../../../bin

include(../common.pri)
