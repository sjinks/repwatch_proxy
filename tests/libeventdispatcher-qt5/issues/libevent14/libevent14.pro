QT       = core testlib
TEMPLATE = app
TARGET   = tst_libevent14
SOURCES += $$PWD/../../../../libs/qt_eventdispatcher_libevent/tests/qt_eventdispatcher_tests/tests-qt5/issues/libevent14/tst_libevent14.cpp
DESTDIR  = ../../../../bin

include(../../common.pri)
