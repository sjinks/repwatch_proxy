HEADERS += \
	$$PWD/../libs/qt_eventdispatcher_libevent/src/eventdispatcher_libevent.h \
	$$PWD/../libs/qt_eventdispatcher_libevent/src/eventdispatcher_libevent_p.h \
	$$PWD/../libs/qt_eventdispatcher_libevent/src/eventdispatcher_libevent_config.h \
	$$PWD/../libs/qt_eventdispatcher_libevent/src/eventdispatcher_libevent_config_p.h \
	$$PWD/../libs/qt_eventdispatcher_libevent/src/libevent2-emul.h \
	$$PWD/../libs/qt_eventdispatcher_libevent/src/qt4compat.h \
	$$PWD/../libs/qt_eventdispatcher_libevent/src/tco.h \
	$$PWD/../libs/qt_eventdispatcher_libevent/src/tco_impl.h

SOURCES += \
	$$PWD/../libs/qt_eventdispatcher_libevent/src/eventdispatcher_libevent.cpp \
	$$PWD/../libs/qt_eventdispatcher_libevent/src/eventdispatcher_libevent_p.cpp \
	$$PWD/../libs/qt_eventdispatcher_libevent/src/timers_p.cpp \
	$$PWD/../libs/qt_eventdispatcher_libevent/src/socknot_p.cpp \
	$$PWD/../libs/qt_eventdispatcher_libevent/src/eventdispatcher_libevent_config.cpp

unix {
	system('cc -E $$PWD/../libs/qt_eventdispatcher_libevent/src/conftests/eventfd.h -o /dev/null 2> /dev/null') {
		SOURCES += $$PWD/../libs/qt_eventdispatcher_libevent/src/tco_eventfd.cpp
	}
	else {
		SOURCES += $$PWD/../libs/qt_eventdispatcher_libevent/src/tco_pipe.cpp
	}

	system('pkg-config --exists libevent') {
		CONFIG    *= link_pkgconfig
		PKGCONFIG += libevent libevent_pthreads
	}
	else {
		system('cc -E $$PWD/../libs/qt_eventdispatcher_libevent/src/conftests/libevent2.h -o /dev/null 2> /dev/null') {
			DEFINES += SJ_LIBEVENT_MAJOR=2
		}
		else:system('cc -E $$PWD/../libs/qt_eventdispatcher_libevent/src/conftests/libevent1.h -o /dev/null 2> /dev/null') {
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
	LIBS += -levent
}

INCLUDEPATH += $$PWD/../libs/qt_eventdispatcher_libevent/src
DEPENDPATH  += $$PWD/../libs/qt_eventdispatcher_libevent/src
