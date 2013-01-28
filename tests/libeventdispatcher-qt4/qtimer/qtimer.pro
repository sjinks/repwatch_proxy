TARGET   = tst_qtimer
QT       = core testlib
HEADERS  = $$PWD/../../../libs/qt_eventdispatcher_libevent/tests-qt4/qtimer/util.h
SOURCES  = $$PWD/../../../libs/qt_eventdispatcher_libevent/tests-qt4/qtimer/tst_qtimer.cpp
DESTDIR  = ../../../bin

include(../common.pri)
