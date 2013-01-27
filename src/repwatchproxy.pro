QT      += network
QT      -= gui
TEMPLATE = app
CONFIG  += release
VERSION  = 0.0.1
TARGET   = repwatchproxy
DESTDIR  = ../bin

HEADERS +=
SOURCES += \
	main.cpp

include(libeventdispatcher.pri)
include(socketconnector.pri)
include(signalwatcher.pri)
