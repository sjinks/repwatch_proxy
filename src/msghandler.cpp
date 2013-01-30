#include <QtCore/QString>
#include "msghandler.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#ifdef Q_OS_UNIX
#	include <syslog.h>
#	include <execinfo.h>
static void* backtrace_buf[4096];
#else
#	include "win_syslog.h"
#endif

void print_backtrace(FILE* outb)
{
#ifdef Q_OS_UNIX
	int stack_size       = backtrace(backtrace_buf, sizeof(backtrace_buf)/sizeof(void*));
	char** stack_symbols = backtrace_symbols(backtrace_buf, stack_size);

	fprintf(outb, "Stack [%d]:\n", stack_size);
	if (FILE* cppfilt = popen("/usr/bin/c++filt", "rw")) {
		dup2(fileno(outb), fileno(cppfilt));
		for (int i = stack_size-1; i>=0; --i) {
			fwrite(stack_symbols[i], 1, strlen(stack_symbols[i]), cppfilt);
		}

		pclose(cppfilt);
	}
	else {
		for (int i = stack_size-1; i>=0; --i) {
			fprintf(outb, "#%d  %p [%s]\n", i, backtrace_buf[i], stack_symbols[i]);
		}
	}
#else
	Q_UNUSED(outb)
#endif
}

#if QT_VERSION < 0x050000
void messageHandler(QtMsgType type, const char* msg)
{
	fprintf(stderr, "%s\n", msg);
	fflush(stderr);

	int level = LOG_DEBUG;

	switch (type) {
		case QtDebugMsg:    level = LOG_DEBUG; break;
		case QtWarningMsg:  level = LOG_WARNING; break;
		case QtCriticalMsg: level = LOG_ERR; break;
		case QtFatalMsg:    level = LOG_CRIT; break;
	}

	syslog(level, "%s", msg);

	if (QtFatalMsg == type) {
		termination_handler();
	}
}
#else
void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	QString severity;
	int level = LOG_DEBUG;

	switch (type) {
		case QtDebugMsg:
			severity = QLatin1String("Debug");
			level = LOG_DEBUG;
			break;

		case QtWarningMsg:
			severity = QLatin1String("Warning");
			level = LOG_WARNING;
			break;

		case QtCriticalMsg:
			severity = QLatin1String("Error");
			level = LOG_ERR;
			break;

		case QtFatalMsg:
			severity = QLatin1String("Fatal");
			level = LOG_CRIT;
	}

#ifdef DEBUG
	QString pattern = QLatin1String("%1: %2 (%3:%4, %5)\n");
	QByteArray message = pattern.arg(severity, msg, QLatin1String(context.file), QString::number(context.line), QLatin1String(context.function)).toLocal8Bit();
#else
	Q_UNUSED(context)
	QString pattern = QLatin1String("%1: %2\n");
	QByteArray message = pattern.arg(severity, msg).toLocal8Bit();
#endif

	fprintf(stderr, "%s", message.constData());
	fflush(stderr);

//	syslog(level, "%s", message.constData());

	if (QtFatalMsg == type) {
		termination_handler();
	}
}

#endif

void sigsegv_handler(int)
{
	termination_handler();
}

void termination_handler(void)
{
	print_backtrace(stderr);
	abort();
}
