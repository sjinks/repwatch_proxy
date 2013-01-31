#include <QtCore/QtEndian>
#include <QtCore/QIODevice>
#include <QtCore/QTimer>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpSocket>
#include "socketconnector.h"
#include "worker.h"
#include "qt4compat.h"

Worker::Worker(QIODevice* peer, QObject* parent)
	: QObject(parent),
	  m_peer(peer), m_target(0), m_connector(0),
	  m_buf(), m_expected_length(-1), m_state(Worker::ConnectedState),
	  m_noauth_allowed(false)
{
	QObject::connect(this->m_peer, SIGNAL(readyRead()), this, SLOT(peerReadyReadHandler()));

	if (qobject_cast<QAbstractSocket*>(this->m_peer)) {
		QObject::connect(this->m_peer, SIGNAL(disconnected()), this, SLOT(disconnectHandler()));
		QObject::connect(this->m_peer, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(peerErrorHandler(QAbstractSocket::SocketError)));
	}
	else {
		QObject::connect(this->m_peer, SIGNAL(aboutToClose()), this, SLOT(disconnectHandler()));
	}
}

Worker::~Worker(void)
{
	qDebug("-%p", this);
}

void Worker::peerReadyReadHandler(void)
{
	QIODevice* peer = qobject_cast<QIODevice*>(this->sender());
	Q_ASSERT(peer != 0);

	if (this->m_state == Worker::ErrorState) {
		// Peer has not disconnected (though it should have) - closing the connection
		Q_EMIT this->error(Worker::UnknownError);
		peer->close();
		return;
	}

	QByteArray buf;
	buf = peer->readAll();
	this->m_buf.append(buf);

	bool cont;
	do {
		cont = false;
		switch(this->m_state) {
			case Worker::ConnectedState:
				cont = this->readGreeting();
				break;

			case Worker::GreetingReceivedState:
				this->parseGreeting();
				break;

			case Worker::AwaitingAuthenticationState:
				this->authenticate();
				break;

			case Worker::AwaitingRequestState:
				this->parseRequest();
				break;

			case Worker::RequestReceivedState:
				this->m_state = Worker::FatalErrorState;
				break;

			case Worker::ConnectionProxiedState:
				Q_ASSERT(this->m_target != 0);
				this->m_target->write(this->m_buf);
				this->m_target->flush();
				this->m_buf.clear();
				break;

			case Worker::ErrorState:
			case Worker::FatalErrorState:
				Q_ASSERT(false);
				break;
		}
	} while (cont);

	if (this->m_state == Worker::FatalErrorState) {
		peer->close();
	}
}

void Worker::targetReadyReadHandler(void)
{
	QByteArray buf = this->m_target->readAll();
	this->writeAndFlush(this->m_peer, buf);
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
	QHostAddress a;
	if (qobject_cast<QAbstractSocket*>(this->m_peer)) {
		a = (qobject_cast<QAbstractSocket*>(this->m_peer))->peerAddress();
	}

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

	if (qobject_cast<QAbstractSocket*>(this->m_peer)) {
		data.port = qToBigEndian<quint16>((qobject_cast<QAbstractSocket*>(this->m_peer))->peerPort());
	}
	else {
		data.port = 0;
	}

	response.append(QByteArray::fromRawData(&data.ptr, sizeof(data.port)));

	QObject::connect(this->m_target, SIGNAL(readyRead()), this, SLOT(targetReadyReadHandler()));
	QObject::connect(this->m_target, SIGNAL(disconnected()), this, SLOT(disconnectHandler()));

	if (this->writeAndFlush(this->m_peer, response) == response.size()) {
		this->m_state = Worker::ConnectionProxiedState;
	}
	else {
		this->m_peer->close();
		this->m_state = Worker::FatalErrorState;
	}
}

void Worker::targetConnectFailureHandler(QAbstractSocket::SocketError e)
{
	char response[] = "\x05\x05\x00\x01\x00\x00\x00\x00\x00\x00"; // Connection refused
	if (e != QAbstractSocket::ConnectionRefusedError) {
		response[1] = '\x02'; // General failure
	}

	this->writeAndFlush(this->m_peer, response, 10);
	this->m_peer->close();
	this->m_state = Worker::FatalErrorState;
}

void Worker::disconnectHandler(void)
{
	Q_EMIT this->connectionClosed();

	this->m_peer->disconnect(this);
	this->m_peer->deleteLater();

	if (this->m_target) {
		this->m_target->disconnect(this);
		this->m_target->deleteLater();
	}

	QObject::connect(this->m_peer, SIGNAL(destroyed()), this, SLOT(deleteLater()), Qt::QueuedConnection);
}

void Worker::peerErrorHandler(QAbstractSocket::SocketError e)
{
	Q_UNUSED(e)
	this->disconnectHandler();
}

/**
 * @see http://tools.ietf.org/html/rfc1928
 *
 * The client connects to the server, and sends a version
 * identifier/method selection message:
 *
 *              +----+----------+----------+
 *              |VER | NMETHODS | METHODS  |
 *              +----+----------+----------+
 *              | 1  |    1     | 1 to 255 |
 *              +----+----------+----------+
 *
 * The VER field is set to X'05' for this version of the protocol. The
 * NMETHODS field contains the number of method identifier octets that
 * appear in the METHODS field.
 */
bool Worker::readGreeting(void)
{
	int size = this->m_buf.size();
	if (-1 == this->m_expected_length) {
		if (size > 1) {
			quint8 ver = static_cast<quint8>(this->m_buf.at(0));
			int len    = static_cast<quint8>(this->m_buf.at(1));

			if (ver != 5) {
				Q_EMIT this->error(Worker::ProtocolVersionMismatch);
				this->m_state = Worker::FatalErrorState;
				return false;
			}

			if (!len) {
				Q_EMIT this->error(Worker::UnknownError);
				this->m_state = Worker::FatalErrorState;
				return false;
			}

			this->m_expected_length = 2 + len;
		}
	}

	if (this->m_expected_length != -1 && size > this->m_expected_length) {
		this->m_state = Worker::FatalErrorState;
		Q_EMIT this->error(Worker::TooMuchData);
		return false;
	}

	if (size == this->m_expected_length) {
		this->m_state = Worker::GreetingReceivedState;
		return true;
	}

	return false;
}

/**
 * @see http://tools.ietf.org/html/rfc1928
 *
 * The server selects from one of the methods given in METHODS, and
 * sends a METHOD selection message:
 *
 *                    +----+--------+
 *                    |VER | METHOD |
 *                    +----+--------+
 *                    | 1  |   1    |
 *                    +----+--------+
 *
 * If the selected METHOD is X'FF', none of the methods listed by the
 * client are acceptable, and the client MUST close the connection.
 *
 * The values currently defined for METHOD are:
 *
 *        o  X'00' NO AUTHENTICATION REQUIRED
 *        o  X'01' GSSAPI
 *        o  X'02' USERNAME/PASSWORD
 *        o  X'03' to X'7F' IANA ASSIGNED
 *        o  X'80' to X'FE' RESERVED FOR PRIVATE METHODS
 *        o  X'FF' NO ACCEPTABLE METHODS
 *
 * The client and server then enter a method-specific sub-negotiation.
 */
void Worker::parseGreeting(void)
{
	char response[] = "\x05\x02"; // Only password authentication accepted

	if (this->m_noauth_allowed && -1 != this->m_buf.indexOf('\x00', 2)) {
		this->m_state = Worker::AwaitingRequestState;
		response[1]   = '\x00'; // Successfully authenticated
	}
	else if (-1 == this->m_buf.indexOf('\x02', 2)) {
		this->m_state = Worker::ErrorState; // Unsupported auth method
		response[1]   = '\xFF';
		Q_EMIT this->error(Worker::UnsupportedAuthMethod);
	}
	else {
		this->m_state = Worker::AwaitingAuthenticationState;
	}

	if (2 != this->writeAndFlush(this->m_peer, response, 2)) {
		this->m_state = Worker::FatalErrorState;
		Q_EMIT this->error(Worker::IOError);
	}

	this->m_expected_length = -1;
	this->m_buf.clear();
}

/**
 * @see http://tools.ietf.org/html/rfc1929
 *
 * Once the SOCKS V5 server has started, and the client has selected the
 * Username/Password Authentication protocol, the Username/Password
 * subnegotiation begins.  This begins with the client producing a
 * Username/Password request:
 *
 *         +----+------+----------+------+----------+
 *         |VER | ULEN |  UNAME   | PLEN |  PASSWD  |
 *         +----+------+----------+------+----------+
 *         | 1  |  1   | 1 to 255 |  1   | 1 to 255 |
 *         +----+------+----------+------+----------+
 *
 * The VER field contains the current version of the subnegotiation,
 * which is X'01'. The ULEN field contains the length of the UNAME field
 * that follows. The UNAME field contains the username as known to the
 * source operating system. The PLEN field contains the length of the
 * PASSWD field that follows. The PASSWD field contains the password
 * association with the given UNAME.
 */
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
				this->m_state = Worker::FatalErrorState;
				Q_EMIT this->error(Worker::ProtocolVersionMismatch);
				return;
			}
		}
	}

	if (this->m_expected_length != -1 && size > this->m_expected_length) {
		this->m_state = Worker::FatalErrorState;
		Q_EMIT this->error(Worker::TooMuchData);
		return;
	}

	if (size == this->m_expected_length) {
		int ulen = static_cast<quint8>(this->m_buf.at(1));
		int plen = static_cast<quint8>(this->m_buf.at(ulen + 2));
		QByteArray username = ulen ? QByteArray(this->m_buf.constData() + 2, ulen) : QByteArray();
		QByteArray password = plen ? QByteArray(this->m_buf.constData() + 2 + ulen + 1, plen) : QByteArray();
		this->m_expected_length = -1;
		this->m_buf.clear();

		if (this->m_noauth_allowed && username.isEmpty() && password.isEmpty()) {
			this->acceptAuthentication();
		}
		else {
			Q_EMIT this->authenticateRequest(username, password);
		}
	}
}

/**
 * @see http://tools.ietf.org/html/rfc1929
 *
 * The server verifies the supplied UNAME and PASSWD, and sends the
 * following response:
 *
 *                      +----+--------+
 *                      |VER | STATUS |
 *                      +----+--------+
 *                      | 1  |   1    |
 *                      +----+--------+
 *
 * A STATUS field of X'00' indicates success. If the server returns a
 * `failure' (STATUS value other than X'00') status, it MUST close the
 * connection.
 */
void Worker::acceptAuthentication(void)
{
	if (this->m_state != Worker::AwaitingAuthenticationState) {
		qWarning("Worker::acceptAuthentication() called while not in AwaitingAuthentication state");
		return;
	}

	char response[] = "\x01\x00";
	this->m_state = Worker::AwaitingRequestState;

	if (2 != this->writeAndFlush(this->m_peer, response, 2)) {
		this->m_state = Worker::FatalErrorState;
		Q_EMIT this->error(Worker::IOError);
	}

}

/**
 * @see http://tools.ietf.org/html/rfc1929
 *
 * The server verifies the supplied UNAME and PASSWD, and sends the
 * following response:
 *
 *                      +----+--------+
 *                      |VER | STATUS |
 *                      +----+--------+
 *                      | 1  |   1    |
 *                      +----+--------+
 *
 * A STATUS field of X'00' indicates success. If the server returns a
 * `failure' (STATUS value other than X'00') status, it MUST close the
 * connection.
 */
void Worker::rejectAuthentication(void)
{
	if (this->m_state != Worker::AwaitingAuthenticationState) {
		qWarning("Worker::rejectAuthentication() called while not in AwaitingAuthentication state");
		return;
	}

	char response[] = "\x01\xFF";
	this->m_state   = Worker::ErrorState;
	if (2 != this->writeAndFlush(this->m_peer, response, 2)) {
		this->m_state = Worker::FatalErrorState;
		Q_EMIT this->error(Worker::IOError);
	}
}

/**
 * @see http://tools.ietf.org/html/rfc1928
 *
 * The SOCKS request is formed as follows:
 *
 *      +----+-----+-------+------+----------+----------+
 *      |VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
 *      +----+-----+-------+------+----------+----------+
 *      | 1  |  1  | X'00' |  1   | Variable |    2     |
 *      +----+-----+-------+------+----------+----------+
 *
 * Where:
 *
 *    o  VER    protocol version: X'05'
 *    o  CMD
 *       o  CONNECT X'01'
 *       o  BIND X'02'
 *       o  UDP ASSOCIATE X'03'
 *    o  RSV    RESERVED
 *    o  ATYP   address type of following address
 *       o  IP V4 address: X'01'
 *       o  DOMAINNAME: X'03'
 *       o  IP V6 address: X'04'
 *    o  DST.ADDR       desired destination address
 *    o  DST.PORT desired destination port in network octet order
 *
 * The SOCKS server will typically evaluate the request based on source
 * and destination addresses, and return one or more reply messages, as
 * appropriate for the request type.
 *
 * In an address field (DST.ADDR, BND.ADDR), the ATYP field specifies
 * the type of address contained within the field:
 *
 *    o  X'01'
 *
 * the address is a version-4 IP address, with a length of 4 octets
 *
 *    o  X'03'
 *
 * the address field contains a fully-qualified domain name.  The first
 * octet of the address field contains the number of octets of name that
 * follow, there is no terminating NUL octet.
 *
 *    o  X'04'
 *
 * the address is a version-6 IP address, with a length of 16 octets.
 */
void Worker::parseRequest(void)
{
	int size = this->m_buf.size();
	if (-1 == this->m_expected_length) {
		if (size > 4) {
			quint8 ver  = static_cast<quint8>(this->m_buf.at(0));
			quint8 rsv  = static_cast<quint8>(this->m_buf.at(2));
			quint8 atyp = static_cast<quint8>(this->m_buf.at(3));

			if (ver != 5) {
				this->m_state = Worker::FatalErrorState;
				Q_EMIT this->error(Worker::ProtocolVersionMismatch);
				return;
			}

			if (rsv != 0) {
				this->m_state = Worker::FatalErrorState;
				Q_EMIT this->error(Worker::UnknownError);
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
					this->m_state = Worker::FatalErrorState;
					Q_EMIT this->error(Worker::UnknownError);
					return;
			}
		}
	}

	if (this->m_expected_length != -1 && size > this->m_expected_length) {
		this->m_state = Worker::FatalErrorState;
		Q_EMIT this->error(Worker::TooMuchData);
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
		this->writeAndFlush(this->m_peer, response, 10);
		this->m_state = Worker::FatalErrorState;
		return;
	}

	this->m_connector = new SocketConnector(this);
	QHostAddress a;
	if (qobject_cast<QAbstractSocket*>(this->m_peer)) {
		a = (qobject_cast<QAbstractSocket*>(this->m_peer))->localAddress();
	}

	if (!this->m_connector->createTcpSocket() || (!a.isNull() && !this->m_connector->bindTo(a))) {
		char response[] = "\x05\x01\x00\x01\x00\x00\x00\x00\x00\x00"; // General SOCKS server failure
		this->writeAndFlush(this->m_peer, response, 10);
		this->m_state = Worker::FatalErrorState;
		return;
	}

	this->m_state = Worker::RequestReceivedState;
	QObject::connect(this->m_connector, SIGNAL(connected()), this, SLOT(targetConnectedHandler()));
	QObject::connect(this->m_connector, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(targetConnectFailureHandler(QAbstractSocket::SocketError)));
	this->m_connector->connectToHost(address, port);
}

qint64 Worker::writeAndFlush(QIODevice* device, const char* buf, int size)
{
	qint64 res = device->write(buf, size);
	QAbstractSocket* sock = qobject_cast<QAbstractSocket*>(device);
	if (sock) {
		sock->flush();
	}

	return res;
}

qint64 Worker::writeAndFlush(QIODevice* device, const QByteArray& buf)
{
	return this->writeAndFlush(device, buf.constData(), buf.size());
}
