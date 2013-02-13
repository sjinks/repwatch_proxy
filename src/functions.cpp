#include <QtCore/QtEndian>
#include <QtNetwork/QHostAddress>
#include <string.h>
#include "functions.h"

bool inSubnet(const QHostAddress& a, const QHostAddress& subnet, int netmask)
{
	QAbstractSocket::NetworkLayerProtocol protocol = a.protocol();

	if (subnet.protocol() != protocol || netmask < 0) {
		return false;
	}

	union {
		quint32 ip;
		quint8 data[4];
	} ip4, net4;

	const quint8* ip;
	const quint8* net;

	if (protocol == QAbstractSocket::IPv4Protocol) {
		if (netmask > 32) {
			netmask = 32;
		}

		ip4.ip  = qToBigEndian(a.toIPv4Address());
		net4.ip = qToBigEndian(subnet.toIPv4Address());
		ip      = ip4.data;
		net     = net4.data;
	}
	else if (protocol == QAbstractSocket::IPv6Protocol) {
		if (netmask > 128) {
			netmask = 128;
		}

		ip  = a.toIPv6Address().c;
		net = subnet.toIPv6Address().c;
	}
	else {
		return false;
	}

	if (netmask >= 8 && memcmp(ip, net, netmask / 8) != 0) {
		return false;
	}

	if ((netmask & 7) == 0) {
		return true;
	}

	// compare the last octet now
	quint8 bytemask = 256 - (1 << (8 - (netmask & 7)));
	quint8 ipbyte   = ip[netmask / 8];
	quint8 netbyte  = net[netmask / 8];
	return (ipbyte & bytemask) == (netbyte & bytemask);
}
