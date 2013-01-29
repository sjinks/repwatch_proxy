#include "myapplication.h"
#include "eventdispatcher_libevent.h"

int main(int argc, char** argv)
{
#if QT_VERSION >= 0x050000
	QCoreApplication::setEventDispatcher(new EventDispatcherLibEvent());
#else
	EventDispatcherLibEvent e;
#endif

	QCoreApplication::setApplicationName(QLatin1String("Proxy"));
	QCoreApplication::setOrganizationName(QLatin1String("Goldbar Enterprises"));
	QCoreApplication::setOrganizationDomain(QLatin1String("goldbar.net"));
#if QT_VERSION >= 0x040400
	QCoreApplication::setApplicationVersion(QLatin1String("0.0.1"));
#endif

	MyApplication app(argc, argv);
	return app.exec();
}
