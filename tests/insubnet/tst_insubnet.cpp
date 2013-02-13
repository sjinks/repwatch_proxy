#include <QtNetwork/QHostAddress>
#include <QtTest/QTest>
#include "functions.h"

Q_DECLARE_METATYPE(QHostAddress)

class InSubnetTest : public QObject {
	Q_OBJECT
public:
	InSubnetTest(void) {}

private Q_SLOTS:
	void testCase1_data(void)
	{
		QTest::addColumn<QString>("address");
		QTest::addColumn<QString>("subnet");
		QTest::addColumn<int>("prefix");
		QTest::addColumn<bool>("result");

		QTest::newRow("self") << QString::fromLatin1("127.0.0.1") << QString::fromLatin1("127.0.0.1") << 32 << true;

		for (int i=31; i>=0; --i) {
			QTest::newRow((QByteArray("self/") + QByteArray::number(i)).constData()) << QString::fromLatin1("128.0.0.1") << QString::fromLatin1("128.0.0.0") << i << true;
		}

		// From http://www.stev.org/post/2012/08/09/C++-Check-an-IP-Address-is-in-a-IPMask-range.aspx
		QTest::newRow("stev1") << QString::fromLatin1("192.168.1.1") << QString::fromLatin1("192.168.1.0") << 24 << true;
		QTest::newRow("stev2") << QString::fromLatin1("192.168.1.1") << QString::fromLatin1("192.168.1.2") << 32 << false;
		QTest::newRow("stev3") << QString::fromLatin1("192.168.1.3") << QString::fromLatin1("192.168.1.2") << 32 << false;
		QTest::newRow("stev4") << QString::fromLatin1("220.1.1.22") << QString::fromLatin1("192.168.1.0") << 24 << false;
		QTest::newRow("stev5") << QString::fromLatin1("220.1.1.22") << QString::fromLatin1("220.1.1.22") << 32 << true;
		QTest::newRow("stev6") << QString::fromLatin1("220.1.1.22") << QString::fromLatin1("220.1.1.23") << 32 << false;
		QTest::newRow("stev7") << QString::fromLatin1("220.1.1.22") << QString::fromLatin1("220.1.1.21") << 32 << false;
		QTest::newRow("stev8") << QString::fromLatin1("0.0.0.1") << QString::fromLatin1("0.0.0.0") << 0 << true;
		QTest::newRow("stev9") << QString::fromLatin1("192.168.1.2") << QString::fromLatin1("10.0.0.1") << 32 << false;
	}

	void testCase1(void)
	{
		QFETCH(QString, address);
		QFETCH(QString, subnet);
		QFETCH(int, prefix);

		QTEST(inSubnet(QHostAddress(address), QHostAddress(subnet), prefix), "result");
	}

	void testQt5_data(void)
	{
		QTest::addColumn<QHostAddress>("address");
		QTest::addColumn<QHostAddress>("subnet");
		QTest::addColumn<int>("prefix");
		QTest::addColumn<bool>("result");

		// invalid QHostAddresses are never in any subnets
		QTest::newRow("invalid_01") << QHostAddress() << QHostAddress() << 32 << false;
		QTest::newRow("invalid_02") << QHostAddress() << QHostAddress(QString("0.0.0.0")) << 32 << false;
		QTest::newRow("invalid_03") << QHostAddress() << QHostAddress(QString("0.0.0.0")) << 8 << false;
		QTest::newRow("invalid_04") << QHostAddress() << QHostAddress(QString("0.0.0.0")) << 0 << false;
		QTest::newRow("invalid_05") << QHostAddress() << QHostAddress("255.255.255.0") << 24 << false;
		QTest::newRow("invalid_06") << QHostAddress() << QHostAddress(QString("::")) << 0 << false;
		QTest::newRow("invalid_07") << QHostAddress() << QHostAddress(QString("::")) << 32 << false;
		QTest::newRow("invalid_08") << QHostAddress() << QHostAddress(QString("::")) << 128<< false;

		// and no host address can be in a subnet whose prefix is invalid
		QTest::newRow("invalid_20") << QHostAddress(QString("0.0.0.0")) << QHostAddress() << 16 << false;
		QTest::newRow("invalid_21") << QHostAddress(QString("::")) << QHostAddress() << 16 << false;
		QTest::newRow("invalid_22") << QHostAddress(QHostAddress::LocalHost) << QHostAddress() << 16 << false;
		QTest::newRow("invalid_23") << QHostAddress(QString("::1")) << QHostAddress() << 16 << false;

		// negative netmasks don't make sense:
		QTest::newRow("invalid_30") << QHostAddress(QString("0.0.0.0")) << QHostAddress(QHostAddress::Any) << -1 << false;
		QTest::newRow("invalid_31") << QHostAddress(QString("::")) << QHostAddress(QString("::")) << -1 << false;

		// we don't support IPv4 belonging in an IPv6 netmask and vice-versa
		QTest::newRow("v4-in-v6") << QHostAddress(QHostAddress::LocalHost) << QHostAddress(QString("::")) << 0 << false;
		QTest::newRow("v6-in-v4") << QHostAddress(QString("::1")) << QHostAddress(QHostAddress::Any) << 0 << false;
		QTest::newRow("v4-in-v6mapped") << QHostAddress(QHostAddress::LocalHost) << QHostAddress("ffff:ffff:ffff:ffff:ffff:ffff:255.0.0.0") << 113 << false;
		QTest::newRow("v4-in-v6mapped2") << QHostAddress(QHostAddress::LocalHost) << QHostAddress("::ffff:255.0.0.0") << 113 << false;

		// IPv4 correct ones
		QTest::newRow("netmask_0") << QHostAddress(QHostAddress::LocalHost) << QHostAddress(QString("0.0.0.0")) << 0 << true;
		QTest::newRow("netmask_0bis") << QHostAddress(QHostAddress::LocalHost) << QHostAddress("255.255.0.0") << 0 << true;
		QTest::newRow("netmask_0ter") << QHostAddress(QHostAddress::LocalHost) << QHostAddress("1.2.3.4") << 0 << true;
		QTest::newRow("netmask_1") << QHostAddress(QHostAddress::LocalHost) << QHostAddress(QString("0.0.0.0")) << 1 << true;
		QTest::newRow("~netmask_1") << QHostAddress(QHostAddress::LocalHost) << QHostAddress("128.0.0.0") << 1 << false;
		QTest::newRow("netmask_1bis") << QHostAddress("224.0.0.1") << QHostAddress("128.0.0.0") << 1 << true;
		QTest::newRow("~netmask_1bis") << QHostAddress("224.0.0.1") << QHostAddress("0.0.0.0") << 1 << false;
		QTest::newRow("netmask_8") << QHostAddress(QHostAddress::LocalHost) << QHostAddress("127.0.0.0") << 8 << true;
		QTest::newRow("~netmask_8") << QHostAddress(QHostAddress::LocalHost) << QHostAddress("126.0.0.0") << 8 << false;
		QTest::newRow("netmask_15") << QHostAddress("10.0.1.255") << QHostAddress("10.0.0.0") << 15 << true;
		QTest::newRow("netmask_16") << QHostAddress("172.16.0.1") << QHostAddress("172.16.0.0") << 16 << true;

		// the address is always in the subnet containing its address, regardless of length:
		QTest::newRow("same_01") << QHostAddress(QHostAddress::LocalHost) << QHostAddress(QHostAddress::LocalHost) << 1 << true;
		QTest::newRow("same_07") << QHostAddress(QHostAddress::LocalHost) << QHostAddress(QHostAddress::LocalHost) << 7 << true;
		QTest::newRow("same_8") << QHostAddress(QHostAddress::LocalHost) << QHostAddress(QHostAddress::LocalHost) << 8 << true;
		QTest::newRow("same_24") << QHostAddress(QHostAddress::LocalHost) << QHostAddress(QHostAddress::LocalHost) << 23 << true;
		QTest::newRow("same_31") << QHostAddress(QHostAddress::LocalHost) << QHostAddress(QHostAddress::LocalHost) << 31 << true;
		QTest::newRow("same_32") << QHostAddress(QHostAddress::LocalHost) << QHostAddress(QHostAddress::LocalHost) << 32 << true;

		// IPv6 correct ones:
		QTest::newRow("ipv6_netmask_0") << QHostAddress(QString("::1")) << QHostAddress(QString("::")) << 0 << true;
		QTest::newRow("ipv6_netmask_0bis") << QHostAddress(QString("::1")) << QHostAddress(QString("::1")) << 0 << true;
		QTest::newRow("ipv6_netmask_0ter") << QHostAddress(QString("::1")) << QHostAddress("ffff::") << 0 << true;
		QTest::newRow("ipv6_netmask_1") << QHostAddress(QString("::1")) << QHostAddress(QString("::")) << 1 << true;
		QTest::newRow("ipv6_netmask_1bis") << QHostAddress("fec0::1") << QHostAddress("8000::") << 1 << true;
		QTest::newRow("~ipv6_netmask_1") << QHostAddress(QString("::1")) << QHostAddress("8000::") << 1 << false;
		QTest::newRow("~ipv6_netmask_1bis") << QHostAddress("fec0::1") << QHostAddress("::") << 1 << false;
		QTest::newRow("ipv6_netmask_47") << QHostAddress("2:3:5::1") << QHostAddress("2:3:4::") << 47 << true;
		QTest::newRow("ipv6_netmask_48") << QHostAddress("2:3:4::1") << QHostAddress("2:3:4::") << 48 << true;
		QTest::newRow("~ipv6_netmask_48") << QHostAddress("2:3:5::1") << QHostAddress("2:3:4::") << 48 << false;
		QTest::newRow("ipv6_netmask_127") << QHostAddress("2:3:4:5::1") << QHostAddress("2:3:4:5::") << 127 << true;
		QTest::newRow("ipv6_netmask_128") << QHostAddress("2:3:4:5::1") << QHostAddress("2:3:4:5::1") << 128 << true;
		QTest::newRow("~ipv6_netmask_128") << QHostAddress("2:3:4:5::1") << QHostAddress("2:3:4:5::0") << 128 << false;
	}

	void testQt5(void)
	{
		QFETCH(QHostAddress, address);
		QFETCH(QHostAddress, subnet);
		QFETCH(int, prefix);

		QTEST(inSubnet(address, subnet, prefix), "result");
	}
};


QTEST_APPLESS_MAIN(InSubnetTest)

#include "tst_insubnet.moc"
