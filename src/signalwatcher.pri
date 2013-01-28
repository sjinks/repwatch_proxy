HEADERS += \
	$$PWD/../libs/qt_signalwatcher/src/signalwatcher.h \
	$$PWD/../libs/qt_signalwatcher/src/helpers_p.h \
	$$PWD/../libs/qt_signalwatcher/src/qt4compat.h

unix:!symbian {
	exists(/usr/include/sys/signalfd.h):     SOURCES += $$PWD/../libs/qt_signalwatcher/src/signalwatcher_signalfd.cpp
	else:exists(/usr/include/sys/eventfd.h): SOURCES += $$PWD/../libs/qt_signalwatcher/src/signalwatcher_eventfd.cpp
	else:                                    SOURCES += $$PWD/../libs/qt_signalwatcher/src/signalwatcher_pipe.cpp
}
else {
	SOURCES += $$PWD/../libs/qt_signalwatcher/src/signalwatcher_none.cpp
}
