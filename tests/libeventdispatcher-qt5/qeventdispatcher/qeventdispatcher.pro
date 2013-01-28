TARGET   = tst_qeventdispatcher
QT       = core testlib
SOURCES  = $$PWD/../../../libs/qt_eventdispatcher_libevent/tests-qt5/qeventdispatcher/tst_qeventdispatcher.cpp
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
DESTDIR  = ../../../bin

include(../common.pri)
