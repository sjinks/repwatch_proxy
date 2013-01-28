CONFIG  += testcase
TARGET   = tst_qsocketnotifier
QT       = core-private network-private testlib network

SOURCES = \
	$$PWD/../../../libs/qt_eventdispatcher_libevent/tests-qt5/qsocketnotifier/tst_qsocketnotifier.cpp \
	$$PWD/../../../libs/qt_eventdispatcher_libevent/tests-qt5/qsocketnotifier/qabstractsocketengine.cpp \
	$$PWD/../../../libs/qt_eventdispatcher_libevent/tests-qt5/qsocketnotifier/qnativesocketengine.cpp \
	$$PWD/../../../libs/qt_eventdispatcher_libevent/tests-qt5/qsocketnotifier/qnativesocketengine_unix.cpp

HEADERS = \
	$$PWD/../../../libs/qt_eventdispatcher_libevent/tests-qt5/qsocketnotifier/qabstractsocketengine_p.h \
	$$PWD/../../../libs/qt_eventdispatcher_libevent/tests-qt5/qsocketnotifier/qnativesocketengine_p.h

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
DESTDIR  = ../../../bin

include(../common.pri)
