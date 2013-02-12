#ifndef MYAPPLICATION_H
#define MYAPPLICATION_H

#include <QtCore/QCoreApplication>
#include <QtCore/QList>

class QHostAddress;
class QSettings;
class QTcpServer;

class MyApplication : public QCoreApplication {
	Q_OBJECT
public:
	MyApplication(int& argc, char** argv);
	virtual ~MyApplication(void);
	int exec(void);

private Q_SLOTS:
	void unixSignalHandler(int sig);
	void newConnectionHandler(void);
	void authenticateRequest(const QByteArray& username, const QByteArray& password, const QByteArray& hostname);

private:
	QSettings* m_settings;
	QList<QTcpServer*> m_servers;

	bool checkAccess(const QHostAddress& remote);
};

#endif // MYAPPLICATION_H
