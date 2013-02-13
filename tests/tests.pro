TEMPLATE = subdirs

SUBDIRS += \
	signalwatcher \
	socketconnector \
	worker \
	insubnet

greaterThan(QT_MAJOR_VERSION, 4) {
	SUBDIRS += libeventdispatcher-qt5
}
else {
	SUBDIRS += libeventdispatcher-qt4
}
