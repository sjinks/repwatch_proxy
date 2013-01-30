#include <QtCore/QString>
#include <cstdarg>
#include "win_syslog.h"

static int g_flags;
static HANDLE g_handle;

void openlog(const char* name, int flags, int)
{
	QString n(name ? QString::fromLocal8Bit(name) : QString());
	wchar_t data[n.size()+1];
	n.toWCharArray(data);

	g_flags  = flags;
	g_handle = RegisterEventSourceW(0, data);
}

void closelog(void)
{
	DeregisterEventSource(g_handle);
}

void syslog(int priority, const char* msg, ...)
{
	std::va_list va;
	va_start(va, msg);
	QString s;
	s.vsprintf(msg, va);
	va_end(va);

	wchar_t data[s.size()+1];
	s.toWCharArray(data);

	ReportEventW(g_handle, priority, 0, 0, NULL, 1, 0, reinterpret_cast<const WCHAR**>(&data), NULL);
}
