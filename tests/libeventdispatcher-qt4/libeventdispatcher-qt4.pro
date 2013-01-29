TEMPLATE = subdirs

SUBDIRS = \
	qeventdispatcher \
	qeventloop \
	qtimer

# qsocketnotifier test may heavily depend on the Qt version

equals(QT_MAJOR_VERSION, 4):greaterThan(QT_MINOR_VERSION, 6) {
	SUBDIRS += events
}
