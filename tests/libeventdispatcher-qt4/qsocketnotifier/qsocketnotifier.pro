CONFIG  += testcase
TARGET   = tst_qsocketnotifier
QT       = core network testlib
HEADERS  = \
	$$PWD/../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt4/qsocketnotifier/private/qabstractsocketengine_p.h \
	$$PWD/../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt4/qsocketnotifier/private/qnativesocketengine_p.h \
	$$PWD/../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt4/qsocketnotifier/private/qnet_unix_p.h

SOURCES  = \
	$$PWD/../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt4/qsocketnotifier/tst_qsocketnotifier.cpp \
	$$PWD/../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt4/qsocketnotifier/qabstractsocketengine.cpp \
	$$PWD/../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt4/qsocketnotifier/qnativesocketengine.cpp \
	$$PWD/../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt4/qsocketnotifier/qnativesocketengine_unix.cpp

DESTDIR  = ../../../bin

INCLUDEPATH += $$PWD/../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt4/qsocketnotifier/private

include(../common.pri)
