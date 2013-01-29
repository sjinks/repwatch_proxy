HEADERS += \
	$$PWD/../../libs/qt_eventdispatcher_libevent/src/eventdispatcher_libevent_config.h \
	$$PWD/../../libs/qt_eventdispatcher_libevent/src/eventdispatcher_libevent_config_p.h \
	$$PWD/../../libs/qt_eventdispatcher_libevent/src/eventdispatcher_libevent.h \
	$$PWD/../../libs/qt_eventdispatcher_libevent/src/eventdispatcher_libevent_p.h \
	$$PWD/../../libs/qt_eventdispatcher_libevent/src/libevent2-emul.h \
	$$PWD/../../libs/qt_eventdispatcher_libevent/src/qt4compat.h

SOURCES += \
	$$PWD/../../libs/qt_eventdispatcher_libevent/src/eventdispatcher_libevent_config.cpp \
	$$PWD/../../libs/qt_eventdispatcher_libevent/src/eventdispatcher_libevent.cpp \
	$$PWD/../../libs/qt_eventdispatcher_libevent/src/eventdispatcher_libevent_p.cpp \
	$$PWD/../../libs/qt_eventdispatcher_libevent/src/socknot_p.cpp \
	$$PWD/../../libs/qt_eventdispatcher_libevent/src/timers_p.cpp

INCLUDEPATH += $$PWD/../../libs/qt_eventdispatcher_libevent/src/
DEPENDPATH  += $$PWD/../../libs/qt_eventdispatcher_libevent/src/

unix {
	system('pkg-config --exists libevent') {
		CONFIG    += link_pkgconfig
		PKGCONFIG += libevent
	}
	else {
		system('cc -E $$PWD/../../libs/qt_eventdispatcher_libevent/src//conftests/libevent2.h -o /dev/null 2> /dev/null') {
			DEFINES += SJ_LIBEVENT_MAJOR=2
		}
		else:system('cc -E $$PWD/../../libs/qt_eventdispatcher_libevent/src//conftests/libevent1.h -o /dev/null 2> /dev/null') {
			DEFINES += SJ_LIBEVENT_MAJOR=1
		}
		else {
			warning("Assuming libevent 1.x")
			DEFINES += SJ_LIBEVENT_MAJOR=1
		}

		LIBS += -levent_core
	}
}
else {
	LIBS += -levent_core
}

*g++*:equals(QT_MAJOR_VERSION, 4):equals(QT_MINOR_VERSION, 2): QMAKE_CXXFLAGS += -Wno-ignored-qualifiers
