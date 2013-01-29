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
	$$PWD/../../src/worker.cpp

include(../../src/socketconnector.pri)

*g++*:equals(QT_MAJOR_VERSION, 4):lessThan(QT_MINOR_VERSION, 5): QMAKE_CXXFLAGS += -Wno-ignored-qualifiers

INCLUDEPATH += $$PWD/../../src
DEPENDPATH  += $$PWD/../../src
