QT      += network testlib
QT      -= gui
TARGET   = tst_socketconnector
CONFIG  += console
CONFIG  -= app_bundle
TEMPLATE = app
DESTDIR  = ../../bin

lessThan(QT_MAJOR_VERSION, 5): CONFIG += qtestlib

SOURCES  = \
	$$PWD/../../libs/qt_socketconnector/tests/socketconnector/tst_socketconnector.cpp \
	$$PWD/../../libs/qt_socketconnector/src/socketconnector.cpp \
	$$PWD/../../libs/qt_socketconnector/src/socketconnector_p.cpp

HEADERS = \
	$$PWD/../../libs/qt_socketconnector/src/qt4compat.h \
	$$PWD/../../libs/qt_socketconnector/src/socketconnector.h \
	$$PWD/../../libs/qt_socketconnector/src/socketconnector_p.h

INCLUDEPATH += $$PWD/../../libs/qt_socketconnector/src/
DEPENDPATH  += $$PWD/../../libs/qt_socketconnector/src/
