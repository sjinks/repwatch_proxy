TARGET  = tst_qeventloop
QT      = core network testlib
HEADERS = $$PWD/../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt4/qeventloop/util.h
SOURCES = $$PWD/../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt4/qeventloop/tst_qeventloop.cpp
DESTDIR = ../../../bin

include(../common.pri)
