QT      += network testlib
QT      -= gui
TARGET   = tst_insubnet
CONFIG  += console
CONFIG  -= app_bundle
TEMPLATE = app
DESTDIR  = ../../bin

lessThan(QT_MAJOR_VERSION, 5): CONFIG += qtestlib

SOURCES += \
	tst_insubnet.cpp \
	$$PWD/../../src/functions.cpp

HEADERS += \
	$$PWD/../../src/functions.h

INCLUDEPATH += $$PWD/../../src
DEPENDPATH  += $$PWD/../../src
