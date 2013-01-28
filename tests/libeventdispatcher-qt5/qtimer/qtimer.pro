TARGET   = tst_qtimer
QT       = core testlib
SOURCES  = $$PWD/../../../libs/qt_eventdispatcher_libevent/tests-qt5/qtimer/tst_qtimer.cpp
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
DESTDIR  = ../../../bin

include(../common.pri)
