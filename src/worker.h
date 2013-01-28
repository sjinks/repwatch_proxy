#ifndef WORKER_H
#define WORKER_H

#include <QtCore/QObject>
#include <QtNetwork/QAbstractSocket>

class QIODevice;
class QTcpSocket;
class SocketConnector;

class Worker : public QObject {
	Q_OBJECT
public:
	Worker(QIODevice* peer, QObject *parent = 0);
	virtual ~Worker(void);

	enum Error {
		NoError,
		ProtocolVersionMismatch,
		TooMuchData,
		UnknownError
	};

	bool noauthAllowed(void) const { return this->m_noauth_allowed; }
	void setNoauthAllowed(bool v)  { this->m_noauth_allowed = v; }

private Q_SLOTS:
	void peerReadyReadHandler(void);
	void targetReadyReadHandler(void);
	void targetConnectedHandler(void);
	void targetConnectFailureHandler(QAbstractSocket::SocketError e);
	void disconnectHandler(void);

Q_SIGNALS:
	void connectionClosed(void);
	void error(Worker::Error e);

private:
	enum State {
		ConnectedState,
		GreetingReceivedState, // Client has sent the greeting
		AwaitingAuthenticationState, // Server has replied with the list of supported authentication methods
		AwaitingRequestState, // Authentication succeeded, waiting for the request
		RequestReceivedState,
		ConnectionProxiedState,
		ErrorState,
		FatalErrorState
	};

	QIODevice* m_peer;
	QTcpSocket* m_target;
	SocketConnector* m_connector;
	QByteArray m_buf;
	int m_expected_length;
	Worker::State m_state;
	bool m_noauth_allowed;

	bool readGreeting(void);
	void parseGreeting(void);
	void authenticate(void);
	void parseRequest(void);

	qint64 writeAndFlush(QIODevice* device, const char* buf, int size);
	qint64 writeAndFlush(QIODevice* device, const QByteArray& buf);

	friend class WorkerTest;
};

#endif // WORKER_H
