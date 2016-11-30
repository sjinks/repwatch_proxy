TARGET   = tst_qsocketnotifier
QT       = core-private network-private testlib network
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
DESTDIR  = ../../../bin
SOURCES  = \
	$$PWD/../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt5/qsocketnotifier-5.7/tst_qsocketnotifier.cpp \
	$$PWD/../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt5/qsocketnotifier-5.7/qabstractsocketengine.cpp \
	$$PWD/../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt5/qsocketnotifier-5.7/qnativesocketengine.cpp
HEADERS  = \
	$$PWD/../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt5/qsocketnotifier-5.7/qabstractsocketengine_p.h \
	$$PWD/../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt5/qsocketnotifier-5.7/qnativesocketengine_p.h

win32: SOURCES += $$PWD/../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt5/qsocketnotifier-5.7/qnativesocketengine_win.cpp
unix:  SOURCES += $$PWD/../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt5/qsocketnotifier-5.7/qnativesocketengine_unix.cpp
unix:  HEADERS += $$PWD/../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt5/qsocketnotifier-5.7/qnet_unix_p.h

include(../common.pri)
