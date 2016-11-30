TARGET   = tst_qsocketnotifier
QT       = core-private network-private testlib network
DESTDIR  = ../../../bin
SOURCES  = \
	$$PWD/../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt5/qsocketnotifier-5.0/tst_qsocketnotifier.cpp \
	$$PWD/../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt5/qsocketnotifier-5.0/qabstractsocketengine.cpp \
	$$PWD/../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt5/qsocketnotifier-5.0/qnativesocketengine.cpp
HEADERS  = \
	$$PWD/../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt5/qsocketnotifier-5.0/qabstractsocketengine_p.h \
	$$PWD/../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt5/qsocketnotifier-5.0/qnativesocketengine_p.h

win32: SOURCES += $$PWD/../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt5/qsocketnotifier-5.0/qnativesocketengine_win.cpp
unix:  SOURCES += $$PWD/../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt5/qsocketnotifier-5.0/qnativesocketengine_unix.cpp

include(../common.pri)
