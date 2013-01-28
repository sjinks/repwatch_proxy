#include <QtCore/QDebug>
#include <QtCore/QtEndian>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpSocket>
#include "socketconnector.h"
#include "worker.h"

Worker::Worker(QTcpSocket* peer, QObject* parent)
	: QObject(parent),
	  m_peer(peer), m_target(0), m_connector(0),
	  m_buf(), m_expected_length(-1), m_state(Worker::Connected)
{
	qDebug("Worker %p created", this);
	QObject::connect(this->m_peer, SIGNAL(readyRead()), this, SLOT(peerReadyReadHandler()));
}

Worker::~Worker(void)
{
	qDebug("Worker %p destroyed", this);
}

void Worker::peerReadyReadHandler(void)
{
	QTcpSocket* peer = qobject_cast<QTcpSocket*>(this->sender());
	Q_ASSERT(peer != 0);

	if (this->m_state == Worker::Error) {
		// Peer has not disconnected - closing the connection
		peer->close();
		return;
	}

	this->m_buf.append(peer->readAll());

	bool cont;
	do {
		cont = false;
		switch(this->m_state) {
			case Worker::Connected:
				cont = this->readGreeting();
				break;

			case Worker::GreetingReceived:
				this->parseGreeting();
				break;

			case Worker::AwaitingAuthentication:
				this->authenticate();
				break;

			case Worker::AwaitingRequest:
				this->parseRequest();
				break;

			case Worker::RequestReceived:
				this->m_state = Worker::FatalError;
				break;

			case Worker::ConnectionProxied:
				Q_ASSERT(this->m_target != 0);
				this->m_target->write(this->m_buf);
				this->m_target->flush();
				this->m_buf.clear();
				break;

			case Worker::Error:
			case Worker::FatalError:
				Q_ASSERT(false);
				break;
		}
	} while (cont);

	if (this->m_state == Worker::FatalError) {
		peer->close();
	}
}

void Worker::targetReadyReadHandler(void)
{
	QByteArray buf = this->m_target->readAll();
	this->m_peer->write(buf);
	this->m_peer->flush();
}

void Worker::targetConnectedHandler(void)
{
	this->m_target = new QTcpSocket(this);
	this->m_connector->assignTo(this->m_target);
	delete this->m_connector;
	this->m_connector = 0;

	union {
		quint32 v4;
		Q_IPV6ADDR v6;
		quint16 port;
		char ptr;
	} data;

	QByteArray response("\x05\x00\x00", 3);
	QHostAddress a = this->m_peer->peerAddress();

	if (a.protocol() == QAbstractSocket::IPv4Protocol) {
		response.append('\x01');
		data.v4 = qToBigEndian(a.toIPv4Address());
		response.append(QByteArray::fromRawData(&data.ptr, sizeof(data.v4)));
	}
	else if (a.protocol() == QAbstractSocket::IPv6Protocol) {
		response.append('\x04');
		data.v6 = a.toIPv6Address();
		response.append(QByteArray::fromRawData(&data.ptr, sizeof(data.v6)));
	}
	else {
		Q_ASSERT(false);
	}

	data.port = qToBigEndian<quint16>(this->m_peer->peerPort());
	response.append(QByteArray::fromRawData(&data.ptr, sizeof(data.port)));

	QObject::connect(this->m_target, SIGNAL(readyRead()), this, SLOT(targetReadyReadHandler()));
	QObject::connect(this->m_target, SIGNAL(disconnected()), this, SLOT(disconnectHandler()));

	if (this->m_peer->write(response) == response.size()) {
		this->m_peer->flush();
		this->m_state = Worker::ConnectionProxied;
	}
	else {
		this->m_peer->abort();
		this->m_state = Worker::FatalError;
	}
}

void Worker::targetConnectFailureHandler(QAbstractSocket::SocketError e)
{
	qDebug() << "Failed to connect to target:" << e;
	char response[] = "\x05\x05\x00\x01\x00\x00\x00\x00\x00\x00"; // Connection refused
	if (e != QAbstractSocket::ConnectionRefusedError) {
		response[1] = '\x02'; // General failure
	}

	this->m_peer->write(response, 10);
	this->m_peer->flush();
	this->m_peer->close();
	this->m_state = Worker::FatalError;
}

void Worker::disconnectHandler(void)
{
	this->m_peer->close();
	this->m_peer->deleteLater();
	this->m_peer->disconnect();

	QObject::connect(this->m_peer, SIGNAL(destroyed()), this, SLOT(deleteLater()), Qt::QueuedConnection);

	if (this->m_target) {
		this->m_target->close();
		this->m_target->deleteLater();
		this->m_target->disconnect();
	}
}

bool Worker::readGreeting(void)
{
	int size = this->m_buf.size();
	if (-1 == this->m_expected_length) {
		if (size > 1) {
			quint8 ver = static_cast<quint8>(this->m_buf.at(0));
			int len    = static_cast<quint8>(this->m_buf.at(1));

			if (ver != 5 || !len) {
				this->m_state = Worker::FatalError;
				return false;
			}

			this->m_expected_length = 2 + len;
		}
	}

	if (size > this->m_expected_length) {
		this->m_state = Worker::FatalError;
		return false;
	}

	if (size == this->m_expected_length) {
		this->m_state = Worker::GreetingReceived;
		return true;
	}

	return false;
}

void Worker::parseGreeting(void)
{
	char response[] = "\x05\x02"; // Only password authentication accepted

	if (-1 != this->m_buf.indexOf('\x00', 2)) {
		this->m_state = Worker::AwaitingRequest; // Unsupported auth method
		response[1]   = '\x00';
	}
	else if (-1 == this->m_buf.indexOf('\x02', 2)) {
		this->m_state = Worker::Error; // Unsupported auth method
		response[1]   = '\xFF';
	}
	else {
		this->m_state = Worker::AwaitingAuthentication;
	}

	if (2 != this->m_peer->write(response, 2)) {
		this->m_state = Worker::FatalError;
	}

	this->m_peer->flush();
	this->m_expected_length = -1;
	this->m_buf.clear();
}

void Worker::authenticate(void)
{
	int size = this->m_buf.size();
	if (-1 == this->m_expected_length) {
		if (size > 1) {
			quint8 ver = static_cast<quint8>(this->m_buf.at(0));
			int ulen   = static_cast<quint8>(this->m_buf.at(1));
			if (size > 2 + ulen) {
				int plen = static_cast<quint8>(this->m_buf.at(ulen + 2));
				this->m_expected_length = ulen + plen + 3;
			}

			if (ver != 1) {
				this->m_state = Worker::FatalError;
				return;
			}
		}
	}

	if (size > this->m_expected_length) {
		this->m_state = Worker::FatalError;
		return;
	}

	int ulen = static_cast<quint8>(this->m_buf.at(1));
	int plen = static_cast<quint8>(this->m_buf.at(ulen + 2));
	QByteArray username = QByteArray::fromRawData(this->m_buf.constData() + 2, ulen);
	QByteArray password = QByteArray::fromRawData(this->m_buf.constData() + 2 + ulen, plen);

	char response[] = "\x01\x00";
	if (username != "repwatch" || password != "repwatch") {
		this->m_state = Worker::Error;
		response[1] = '\xFF';
	}
	else {
		this->m_state = Worker::AwaitingRequest;
	}

	if (2 != this->m_peer->write(response, 2)) {
		this->m_state = Worker::FatalError;
	}

	this->m_peer->flush();
	this->m_expected_length = -1;
	this->m_buf.clear();
}

void Worker::parseRequest(void)
{
	int size = this->m_buf.size();
	if (-1 == this->m_expected_length) {
		if (size > 4) {
			quint8 ver  = static_cast<quint8>(this->m_buf.at(0));
			quint8 rsv  = static_cast<quint8>(this->m_buf.at(2));
			quint8 atyp = static_cast<quint8>(this->m_buf.at(3));

			if (ver != 5 || rsv != 0) {
				this->m_state = Worker::FatalError;
				return;
			}

			switch (atyp) {
				case 1: // IPv4 address
					this->m_expected_length = 10;
					break;

				case 4: // IPv6 address
					this->m_expected_length = 22;
					break;

				case 3: // Domain name
					this->m_expected_length = 7 + static_cast<quint8>(this->m_buf.at(4));
					break;

				default:
					this->m_state = Worker::FatalError;
					return;
			}
		}
	}

	if (size > this->m_expected_length) {
		this->m_state = Worker::FatalError;
		return;
	}

	QString address;
	quint16 port = 0;
	quint8 cmd  = static_cast<quint8>(this->m_buf.at(1));
	quint8 atyp = static_cast<quint8>(this->m_buf.at(3));
	switch (atyp) {
		case 1: {
			QHostAddress a;
			quint32 addr;
			memcpy(&addr, this->m_buf.constData() + 4, sizeof(quint32));
			memcpy(&port, this->m_buf.constData() + 8, sizeof(quint16));
			a.setAddress(qFromBigEndian(addr));
			address = a.toString();
			break;
		}

		case 4: {
			QHostAddress a;
			quint8 addr[16];
			memcpy(addr, this->m_buf.constData() + 4, 16);
			memcpy(&port, this->m_buf.constData() + 20, sizeof(quint16));
			a.setAddress(addr);
			address = a.toString();
			break;
		}

		case 3: {
			int len = static_cast<quint8>(this->m_buf.at(4));
			QByteArray tmp = QByteArray::fromRawData(this->m_buf.constData() + 5, len);
			address = QString::fromLocal8Bit(tmp.constData(), len);
			memcpy(&port, this->m_buf.constData() + this->m_expected_length - 2, sizeof(quint16));
			break;
		}

		default:
			Q_ASSERT(false);
			break;
	}

	this->m_expected_length = -1;
	this->m_buf.clear();

	port = qFromBigEndian(port);

	if (cmd != 1) { // Connect
		char response[] = "\x05\x07\x00\x01\x00\x00\x00\x00\x00\x00"; // Command not supported
		this->m_peer->write(response, 10);
		this->m_peer->flush();
		this->m_state = Worker::FatalError;
		return;
	}

	this->m_connector = new SocketConnector(this);
	if (!this->m_connector->createTcpSocket() || !this->m_connector->bindTo(this->m_peer->localAddress())) {
		char response[] = "\x05\x01\x00\x01\x00\x00\x00\x00\x00\x00"; // General SOCKS server failure
		this->m_peer->write(response, 10);
		this->m_peer->flush();
		this->m_state = Worker::FatalError;
		return;
	}

	this->m_state = Worker::RequestReceived;
	QObject::connect(this->m_connector, SIGNAL(connected()), this, SLOT(targetConnectedHandler()));
	QObject::connect(this->m_connector, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(targetConnectFailureHandler(QAbstractSocket::SocketError)));
	qDebug() << address << port;
	this->m_connector->connectToHost(address, port);
}
