HEADERS += \
	$$PWD/../libs/qt_signalwatcher/src/signalwatcher.h \
	$$PWD/../libs/qt_signalwatcher/src/helpers_p.h \
	$$PWD/../libs/qt_signalwatcher/src/qt4compat.h

unix:!symbian {
	system('cc -E $$PWD/../libs/qt_signalwatcher/src/conftests/signalfd.h -o /dev/null 2> /dev/null') {
		SOURCES += $$PWD/../libs/qt_signalwatcher/src/signalwatcher_signalfd.cpp
	}
	else:system('cc -E $$PWD/../libs/qt_signalwatcher/src/conftests/eventfd.h -o /dev/null 2> /dev/null'): {
		SOURCES += $$PWD/../libs/qt_signalwatcher/src/signalwatcher_eventfd.cpp
	}
	else {
		SOURCES += $$PWD/../libs/qt_signalwatcher/src/signalwatcher_pipe.cpp
	}
}
else {
	SOURCES += $$PWD/../libs/qt_signalwatcher/src/signalwatcher_none.cpp
}

INCLUDEPATH += $$PWD/../libs/qt_signalwatcher/src
DEPENDPATH  += $$PWD/../libs/qt_signalwatcher/src
