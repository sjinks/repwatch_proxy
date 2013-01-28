TARGET  = tst_qeventloop
QT      = core network testlib core-private
SOURCES = $$PWD/../../../libs/qt_eventdispatcher_libevent/tests-qt5/qeventloop/tst_qeventloop.cpp
DESTDIR = ../../../bin

include(../common.pri)
