QT      += network
QT      -= gui
TEMPLATE = app
CONFIG  += hide_symbols release
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

HEADERS +=
SOURCES += \
	main.cpp

include(libeventdispatcher.pri)
include(socketconnector.pri)
include(signalwatcher.pri)
