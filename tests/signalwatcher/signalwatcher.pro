QT      += testlib
QT      -= gui
TARGET   = tst_signalwatcher
CONFIG  += console
CONFIG  -= app_bundle
TEMPLATE = app
DESTDIR  = ../../bin

lessThan(QT_MAJOR_VERSION, 5): CONFIG += qtestlib

HEADERS += \
	$$PWD/../../libs/qt_signalwatcher/tests/signalwatchertest.h \
	$$PWD/../../libs/qt_signalwatcher/src/helpers_p.h \
	$$PWD/../../libs/qt_signalwatcher/src/qt4compat.h \
	$$PWD/../../libs/qt_signalwatcher/src/signalwatcher.h

SOURCES += \
	$$PWD/../../libs/qt_signalwatcher/tests/signalwatchertest.cpp \
	$$PWD/../../libs/qt_signalwatcher/tests/main.cpp

unix:!symbian {
	system('cc -E $$PWD/../../libs/qt_signalwatcher/src/conftests/signalfd.h -o /dev/null 2> /dev/null') {
		SOURCES += $$PWD/../../libs/qt_signalwatcher/src/signalwatcher_signalfd.cpp
	}
	else:system('cc -E $$PWD/../../libs/qt_signalwatcher/src/conftests/eventfd.h -o /dev/null 2> /dev/null') {
		SOURCES += $$PWD/../../libs/qt_signalwatcher/src/signalwatcher_eventfd.cpp
	}
	else {
		SOURCES += $$PWD/../../libs/qt_signalwatcher/src/signalwatcher_pipe.cpp
	}
}
else {
	SOURCES += $$PWD/../../libs/qt_signalwatcher/src/signalwatcher_none.cpp
}

INCLUDEPATH += $$PWD/../../libs/qt_signalwatcher/src/
DEPENDPATH  += $$PWD/../../libs/qt_signalwatcher/src/

*g++*:equals(QT_MAJOR_VERSION, 4):lessThan(QT_MINOR_VERSION, 5): QMAKE_CXXFLAGS += -Wno-ignored-qualifiers
