QT      += network
QT      -= gui
TEMPLATE = app
CONFIG  += console hide_symbols debug
CONFIG  -= app_bundle
VERSION  = 0.0.1
TARGET   = repwatchproxy
DESTDIR  = ../bin

DEFINES *= \
	QT_NO_CAST_FROM_ASCII \
	QT_NO_CAST_TO_ASCII \
	QT_NO_CAST_FROM_BYTEARRAY \
	QT_ASCII_CAST_WARNINGS \
	QT_NO_URL_CAST_FROM_STRING \
	QT_DISABLE_DEPRECATED_BEFORE=0x050100

HEADERS += \
	myapplication.h \
	worker.h

SOURCES += \
	main.cpp \
	myapplication.cpp \
	worker.cpp

DEFINES += REPWATCHPROXY_VERSION=$$VERSION

*g++*:equals(QT_MAJOR_VERSION, 4):lessThan(QT_MINOR_VERSION, 5): QMAKE_CXXFLAGS += -Wno-ignored-qualifiers

include(libeventdispatcher.pri)
include(socketconnector.pri)
include(signalwatcher.pri)
