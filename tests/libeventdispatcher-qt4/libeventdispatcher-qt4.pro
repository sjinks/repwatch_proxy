TEMPLATE = subdirs

SUBDIRS = \
	qeventdispatcher \
	qeventloop \
	qtimer \
	events

# qsocketnotifier test may heavily depend on the Qt version
