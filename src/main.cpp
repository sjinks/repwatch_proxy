#include "myapplication.h"
#include "eventdispatcher_libevent.h"
#include "msghandler.h"
#include "qt4compat.h"

int main(int argc, char** argv)
{
#if QT_VERSION < 0x050000
	qInstallMsgHandler(messageHandler);
#else
	qInstallMessageHandler(messageHandler);
#endif

#if QT_VERSION >= 0x050000
	QCoreApplication::setEventDispatcher(new EventDispatcherLibEvent());
#else
	EventDispatcherLibEvent e;
#endif

	QCoreApplication::setApplicationName(QLatin1String("Proxy"));
	QCoreApplication::setOrganizationName(QLatin1String("Goldbar Enterprises"));
	QCoreApplication::setOrganizationDomain(QLatin1String("goldbar.net"));
#if QT_VERSION >= 0x040400
#if defined(REPWATCHPROXY_VERSION)
#define RPV QT_STRINGIFY(REPWATCHPROXY_VERSION)
	QCoreApplication::setApplicationVersion(QLatin1String(RPV));
#undef RPV
#else
	QCoreApplication::setApplicationVersion(QLatin1String("0.0.1"));
#endif
#endif

	MyApplication app(argc, argv);
	return app.exec();
}
