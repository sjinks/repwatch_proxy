TEMPLATE = subdirs
CONFIG  += ordered
SUBDIRS  = \
	client \
	tests

client.file = src/repwatchproxy.pro
tests.file  = tests/tests.pro
