#ifndef WORKER_H
#define WORKER_H

#include <QtCore/QObject>
#include <QtNetwork/QAbstractSocket>

class QTcpSocket;
class SocketConnector;

class Worker : public QObject {
	Q_OBJECT
public:
	Worker(QTcpSocket* peer, QObject *parent = 0);
	virtual ~Worker(void);

private Q_SLOTS:
	void peerReadyReadHandler(void);
	void targetReadyReadHandler(void);
	void targetConnectedHandler(void);
	void targetConnectFailureHandler(QAbstractSocket::SocketError e);
	void disconnectHandler(void);

private:
	enum State {
		Connected,
		GreetingReceived, // Client has sent the greeting
		AwaitingAuthentication, // Server has replied with the list of supported authentication methods
		AwaitingRequest, // Authentication succeeded, waiting for the request
		RequestReceived,
		ConnectionProxied,
		Error,
		FatalError
	};

	QTcpSocket* m_peer;
	QTcpSocket* m_target;
	SocketConnector* m_connector;
	QByteArray m_buf;
	int m_expected_length;
	Worker::State m_state;

	bool readGreeting(void);
	void parseGreeting(void);
	void authenticate(void);
	void parseRequest(void);
};

#endif // WORKER_H
