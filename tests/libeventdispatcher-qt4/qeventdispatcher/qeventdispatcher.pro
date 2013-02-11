TARGET   = tst_qeventdispatcher
TEMPLATE = app
QT       = core testlib
SOURCES  = $$PWD/../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt4/qeventdispatcher/tst_qeventdispatcher.cpp
DESTDIR  = ../../../bin

include(../common.pri)
