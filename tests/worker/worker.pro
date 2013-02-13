QT      += network testlib
QT      -= gui
TEMPLATE = app
TARGET   = tst_workertest
CONFIG  += console
CONFIG  -= app_bundle
DESTDIR  = ../../bin

lessThan(QT_MAJOR_VERSION, 5): CONFIG += qtestlib

HEADERS = \
	workertest.h \
	$$PWD/../../src/worker.h

SOURCES = \
	tst_workertest.cpp \
	workertest.cpp \
	$$PWD/../../src/worker.cpp \
	$$PWD/../../libs/qt_socketconnector/src/socketconnector.cpp \
	$$PWD/../../libs/qt_socketconnector/src/socketconnector_p.cpp

INCLUDEPATH += $$PWD/../../src $$OUT_PWD/../../src
DEPENDPATH  += $$PWD/../../src $$OUT_PWD/../../src

INCLUDEPATH += $$PWD/../../libs/qt_socketconnector/src
DEPENDPATH  += $$PWD/../../libs/qt_socketconnector/src
