#include <QtCore/QFile>
#include <QtCore/QSettings>
#include <QtCore/QStringList>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <signal.h>
#include "myapplication.h"
#include "signalwatcher.h"
#include "worker.h"

MyApplication::MyApplication(int &argc, char **argv)
	: QCoreApplication(argc, argv), m_settings(0), m_servers()
{
	QStringList args = QCoreApplication::arguments();
	if (args.size() >= 2 && QFile::exists(args.at(1))) {
		this->m_settings = new QSettings(args.at(1), QSettings::IniFormat, this);
	}
	else {
		this->m_settings = new QSettings(this);
	}

#ifdef Q_OS_UNIX
	SignalWatcher* w = SignalWatcher::instance();
	QObject::connect(w, SIGNAL(unixSignal(int)), this, SLOT(unixSignalHandler(int)));
	w->watch(SIGTERM);
	w->watch(SIGINT);
	w->watch(SIGQUIT);
	w->watch(SIGHUP);
#endif
}

MyApplication::~MyApplication(void)
{
}

int MyApplication::exec(void)
{
	QHostAddress a(QHostAddress::Any);
	quint16 port = 39999;

	QTcpServer* server = new QTcpServer(this);
	if (!server->listen(a, port)) {
		qWarning("Failed to start a server on %s:%d: %s", qPrintable(a.toString()), int(port), qPrintable(server->errorString()));
		delete server;
	}
	else {
		server->setMaxPendingConnections(128);
		QObject::connect(server, SIGNAL(newConnection()), this, SLOT(newConnectionHandler()));
		this->m_servers.append(server);
	}

	if (this->m_servers.isEmpty()) {
		qCritical("Fatal error: no listening sockets available");
		return 1;
	}

	return QCoreApplication::exec();
}

void MyApplication::unixSignalHandler(int sig)
{
#ifdef Q_OS_UNIX
	switch (sig) {
		case SIGTERM:
		case SIGINT:
		case SIGQUIT:
			// terminate
			QCoreApplication::quit();
			break;

		case SIGHUP:
			// reload
			break;

		default:
			break;
	}
#else
	Q_UNUSED(sig)
#endif
}

void MyApplication::newConnectionHandler(void)
{
	QTcpServer* server = qobject_cast<QTcpServer*>(this->sender());
	if (server && server->hasPendingConnections()) {
		QTcpSocket* socket = server->nextPendingConnection();
		Q_ASSERT(socket != 0);

		Worker* w = new Worker(socket, this);
		w->setNoauthAllowed(true);
	}
}
