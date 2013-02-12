#include <QtCore/QFile>
#include <QtCore/QSettings>
#include <QtCore/QStringList>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <signal.h>
#include "myapplication.h"
#include "signalwatcher.h"
#include "worker.h"

#ifdef HAVE_PAM
#include "pamauthenticator.h"
#endif

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
	int port              = this->m_settings->value(QLatin1String("server/proxyport"), 39999).toInt();
	QStringList addresses = this->m_settings->value(QLatin1String("server/listen")).toStringList();
	if (addresses.isEmpty()) {
		addresses.append(QHostAddress(QHostAddress::Any).toString());
	}

	if (port < 1 || port > 65535) {
		port = 39999;
	}

	for (int i=0; i<addresses.size(); ++i) {
		QHostAddress a(addresses.at(i));

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
			// reload settiblongs
			this->m_settings->sync();
			// reload app
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
//		w->setNoauthAllowed(true);
		QObject::connect(w, SIGNAL(authenticateRequest(QByteArray,QByteArray,QByteArray)), this, SLOT(authenticateRequest(QByteArray,QByteArray,QByteArray)));
	}
}

void MyApplication::authenticateRequest(const QByteArray& username, const QByteArray& password, const QByteArray& hostname)
{
	Worker* w = qobject_cast<Worker*>(this->sender());
	if (!w) {
		return;
	}

#ifdef HAVE_PAM
	PAMAuthenticator* a = new PAMAuthenticator(username, password, hostname, w);
	if (a->authenticate()) {
		w->acceptAuthentication();
	}
	else {
		w->rejectAuthentication();
		delete a;
	}

	return;
#endif

	w->acceptAuthentication();
}
