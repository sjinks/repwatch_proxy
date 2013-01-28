TEMPLATE = subdirs

SUBDIRS += \
	signalwatcher \
	socketconnector \
    worker

greaterThan(QT_MAJOR_VERSION, 4) {
	SUBDIRS += libeventdispatcher-qt5
}
else:equals(QT_MAJOR_VERSION, 4):greaterThan(QT_MINOR_VERSION, 5) {
	SUBDIRS += libeventdispatcher-qt4
}
