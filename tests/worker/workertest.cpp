#include <QtCore/QBuffer>
#include <QtCore/QCoreApplication>
#include <QtCore/QPointer>
#include <QtCore/QVariant>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>
#include "workertest.h"
#include "worker.h"

Q_DECLARE_METATYPE(Worker::Error)

WorkerTest::WorkerTest(void)
	: input(0), worker(0)
{
}

void WorkerTest::writeData(const QByteArray& data)
{
	QCOMPARE(int(this->input->write(data)), data.size());
	this->input->seek(this->input->size() - data.size());
	QCoreApplication::processEvents();
}

void WorkerTest::initTestCase(void)
{
	qRegisterMetaType<Worker::Error>("Worker::Error");
}

void WorkerTest::cleanupTestCase(void)
{
}

void WorkerTest::init(void)
{
	this->input = new QBuffer();
	QVERIFY(this->input->open(QIODevice::ReadWrite));

	this->worker = new Worker(this->input, this);
}

void WorkerTest::cleanup(void)
{
	delete this->worker;
	this->worker = 0;
	delete this->input;
	this->input = 0;
}

void WorkerTest::testGreeting(void)
{
	QCOMPARE(this->worker->m_peer, this->input);
	QCOMPARE(this->worker->m_state, Worker::ConnectedState);
	QVERIFY(!this->worker->m_target);
	QVERIFY(!this->worker->m_connector);
	QVERIFY(this->worker->m_buf.isEmpty());
	QCOMPARE(this->worker->m_expected_length, -1);

	this->writeData("\x05");
	QVERIFY(!this->worker->m_target);
	QVERIFY(!this->worker->m_connector);
	QCOMPARE(this->worker->m_buf.size(), 1);
	QCOMPARE(this->worker->m_expected_length, -1);

	this->writeData("\x01");
	QVERIFY(!this->worker->m_target);
	QVERIFY(!this->worker->m_connector);
	QCOMPARE(this->worker->m_buf.size(), 2);
	QCOMPARE(this->worker->m_expected_length, 3);

	int pos = this->input->pos() + 1;
	this->writeData("\x02");
	QVERIFY(!this->worker->m_target);
	QVERIFY(!this->worker->m_connector);
	QCOMPARE(this->worker->m_buf.size(), 0);
	QCOMPARE(this->worker->m_expected_length, -1);
	QCOMPARE(this->worker->m_state, Worker::AwaitingAuthenticationState);

	QCoreApplication::processEvents();
	QCOMPARE(int(this->input->pos()) - pos, 2);
	QVERIFY(this->input->seek(pos));

	QByteArray buf = this->input->readAll();
	QCOMPARE(buf.size(), 2);
	QCOMPARE(buf.at(0), '\x05');
	QCOMPARE(buf.at(1), '\x02');

	QPointer<QBuffer> b(this->input);
	QPointer<Worker> w(this->worker);

	QSignalSpy spy(this->worker, SIGNAL(connectionClosed()));
	QSignalSpy spy2(this->input, SIGNAL(destroyed()));

	this->input->close();
	QCOMPARE(spy.count(), 1);

	QVERIFY(!b.isNull());
	delete this->input;
	this->input = 0;
	QVERIFY(b.isNull());

	QCoreApplication::processEvents();
	QCOMPARE(spy2.count(), 1);

	QCoreApplication::processEvents();
	QVERIFY(w.isNull());
	this->worker = 0;
}

void WorkerTest::testGreetingProtocolFailure1(void)
{
	QSignalSpy spy(this->worker, SIGNAL(error(Worker::Error)));

	this->writeData("\x01");
	QVERIFY(!this->worker->m_target);
	QVERIFY(!this->worker->m_connector);
	QCOMPARE(this->worker->m_buf.size(), 1);
	QCOMPARE(this->worker->m_expected_length, -1);

	this->writeData("\xFF");
	QVERIFY(!this->worker->m_target);
	QVERIFY(!this->worker->m_connector);
	QCOMPARE(this->worker->m_buf.size(), 2);
	QCOMPARE(this->worker->m_state, Worker::FatalErrorState);

	QCOMPARE(spy.count(), 1);
	QVariantList args = spy.takeFirst();
	QCOMPARE(qvariant_cast<Worker::Error>(args.at(0)), Worker::ProtocolVersionMismatch);
}

void WorkerTest::testGreetingProtocolFailure2(void)
{
	QSignalSpy spy(this->worker, SIGNAL(error(Worker::Error)));

	this->writeData("\x05");
	QVERIFY(!this->worker->m_target);
	QVERIFY(!this->worker->m_connector);
	QCOMPARE(this->worker->m_buf.size(), 1);
	QCOMPARE(this->worker->m_expected_length, -1);

	this->writeData(QByteArray("\x00", 1));
	QVERIFY(!this->worker->m_target);
	QVERIFY(!this->worker->m_connector);
	QCOMPARE(this->worker->m_buf.size(), 2);
	QCOMPARE(this->worker->m_state, Worker::FatalErrorState);

	QCOMPARE(spy.count(), 1);
	QVariantList args = spy.takeFirst();
	QCOMPARE(qvariant_cast<Worker::Error>(args.at(0)), Worker::UnknownError);
}

void WorkerTest::testGreetingTooMuchData(void)
{
	QSignalSpy spy(this->worker, SIGNAL(error(Worker::Error)));

	this->writeData("\x05\x01\x01\x01");
	QCOMPARE(this->worker->m_state, Worker::FatalErrorState);
	QCOMPARE(spy.count(), 1);
	QVariantList args = spy.takeFirst();
	QCOMPARE(qvariant_cast<Worker::Error>(args.at(0)), Worker::TooMuchData);
}

void WorkerTest::testNoAuthSuccess(void)
{
	this->worker->setNoauthAllowed(true);
	this->writeData(QByteArray("\x05\x01\x00", 3));
	QCOMPARE(this->worker->m_state, Worker::AwaitingRequestState);

	QByteArray buf;
	QVERIFY(this->input->seek(3));
	buf = this->input->readAll();
	QCOMPARE(buf.size(), 2);
	QCOMPARE(buf.at(0), '\x05');
	QCOMPARE(buf.at(1), '\x00');
}

void WorkerTest::testNoAuthFailure(void)
{
	QVERIFY(!this->worker->noauthAllowed());
	this->writeData(QByteArray("\x05\x01\x00", 3));
	QCOMPARE(this->worker->m_state, Worker::ErrorState);

	QByteArray buf;
	QVERIFY(this->input->seek(3));
	buf = this->input->readAll();
	QCOMPARE(buf.size(), 2);
	QCOMPARE(buf.at(0), '\x05');
	QVERIFY(buf.at(1) != '\x00');
}

void WorkerTest::testUnsupportedAuthMethod(void)
{
	QSignalSpy spy(this->worker, SIGNAL(error(Worker::Error)));

	this->writeData(QByteArray("\x05\x01\xFF", 3));
	QCOMPARE(this->worker->m_state, Worker::ErrorState);

	QByteArray buf;
	QVERIFY(this->input->seek(3));
	buf = this->input->readAll();
	QCOMPARE(buf.size(), 2);
	QCOMPARE(buf.at(0), '\x05');
	QCOMPARE(buf.at(1), '\xFF');

	QCOMPARE(spy.count(), 1);
	QVariantList args = spy.takeFirst();
	QCOMPARE(qvariant_cast<Worker::Error>(args.at(0)), Worker::UnsupportedAuthMethod);
}

void WorkerTest::testAuthTooMuchData(void)
{
	QSignalSpy spy(this->worker, SIGNAL(error(Worker::Error)));

	this->writeData("\x05\x01\x02");
	QCOMPARE(this->worker->m_state, Worker::AwaitingAuthenticationState);
	QCOMPARE(spy.count(), 0);

	QByteArray buf;
	QVERIFY(this->input->seek(3));
	buf = this->input->readAll();
	QCOMPARE(buf.size(), 2);
	QCOMPARE(buf.at(0), '\x05');
	QCOMPARE(buf.at(1), '\x02');

	this->writeData("\x01\x01\x50\x01\x50\xFF");
	QCOMPARE(this->worker->m_state, Worker::FatalErrorState);
	QCOMPARE(spy.count(), 1);
	QVariantList args = spy.takeFirst();
	QCOMPARE(qvariant_cast<Worker::Error>(args.at(0)), Worker::TooMuchData);
}

void WorkerTest::testAuthBadVersion(void)
{
	QSignalSpy spy(this->worker, SIGNAL(error(Worker::Error)));

	this->writeData("\x05\x01\x02");
	QCOMPARE(this->worker->m_state, Worker::AwaitingAuthenticationState);
	QCOMPARE(spy.count(), 0);

	QByteArray buf;
	QVERIFY(this->input->seek(3));
	buf = this->input->readAll();
	QCOMPARE(buf.size(), 2);
	QCOMPARE(buf.at(0), '\x05');
	QCOMPARE(buf.at(1), '\x02');

	this->writeData("\xFF\x01\x50");
	QCOMPARE(this->worker->m_state, Worker::FatalErrorState);
	QCOMPARE(spy.count(), 1);
	QVariantList args = spy.takeFirst();
	QCOMPARE(qvariant_cast<Worker::Error>(args.at(0)), Worker::ProtocolVersionMismatch);
}

void WorkerTest::testRejectAuth(void)
{
	QSignalSpy spy(this->worker, SIGNAL(authenticateRequest(QByteArray,QByteArray)));

	this->writeData("\x05\x01\x02");
	QCOMPARE(this->worker->m_state, Worker::AwaitingAuthenticationState);

	QByteArray buf;
	QVERIFY(this->input->seek(3));
	buf = this->input->readAll();
	QCOMPARE(buf.size(), 2);
	QCOMPARE(buf.at(0), '\x05');
	QCOMPARE(buf.at(1), '\x02');

	this->writeData("\x01\x01\x50\x01\x51");
	QCOMPARE(spy.count(), 1);
	QCOMPARE(spy.first().size(), 2);

	QVariantList args = spy.takeFirst();
	QVariant u = args.at(0);
	QVariant p = args.at(1);
	QVERIFY(u.type() == QVariant::ByteArray);
	QVERIFY(p.type() == QVariant::ByteArray);

	QVERIFY(u.toByteArray() == QByteArray("\x50"));
	QVERIFY(p.toByteArray() == QByteArray("\x51"));

	QCOMPARE(this->worker->m_state, Worker::AwaitingAuthenticationState);
	int pos = this->input->pos();

	this->worker->rejectAuthentication();
	QCOMPARE(this->worker->m_state, Worker::ErrorState);
	QVERIFY(this->input->seek(pos));

	buf = this->input->readAll();
	QCOMPARE(buf.size(), 2);
	QCOMPARE(buf.at(0), '\x01');
	QVERIFY(buf.at(1) != '\x00');
}

void WorkerTest::testAcceptAuth(void)
{
	QSignalSpy spy(this->worker, SIGNAL(authenticateRequest(QByteArray,QByteArray)));

	this->writeData("\x05\x01\x02");
	QCOMPARE(this->worker->m_state, Worker::AwaitingAuthenticationState);

	QByteArray buf;
	QVERIFY(this->input->seek(3));
	buf = this->input->readAll();
	QCOMPARE(buf.size(), 2);
	QCOMPARE(buf.at(0), '\x05');
	QCOMPARE(buf.at(1), '\x02');

	this->writeData("\x01\x01\x50\x01\x51");
	QCOMPARE(spy.count(), 1);
	QCOMPARE(spy.first().size(), 2);

	QVariantList args = spy.takeFirst();
	QVariant u = args.at(0);
	QVariant p = args.at(1);
	QVERIFY(u.type() == QVariant::ByteArray);
	QVERIFY(p.type() == QVariant::ByteArray);

	QVERIFY(u.toByteArray() == QByteArray("\x50"));
	QVERIFY(p.toByteArray() == QByteArray("\x51"));

	QCOMPARE(this->worker->m_state, Worker::AwaitingAuthenticationState);
	int pos = this->input->pos();

	this->worker->acceptAuthentication();
	QCOMPARE(this->worker->m_state, Worker::AwaitingRequestState);
	QVERIFY(this->input->seek(pos));

	buf = this->input->readAll();
	QCOMPARE(buf.size(), 2);
	QCOMPARE(buf.at(0), '\x01');
	QCOMPARE(buf.at(1), '\x00');
}

void WorkerTest::testZeroLengthAuth(void)
{
	QSignalSpy spy(this->worker, SIGNAL(authenticateRequest(QByteArray,QByteArray)));

	this->writeData("\x05\x01\x02");
	QCOMPARE(this->worker->m_state, Worker::AwaitingAuthenticationState);

	QByteArray buf;
	QVERIFY(this->input->seek(3));
	buf = this->input->readAll();
	QCOMPARE(buf.size(), 2);
	QCOMPARE(buf.at(0), '\x05');
	QCOMPARE(buf.at(1), '\x02');

	this->writeData(QByteArray("\x01\x00\x00", 3));
	QCOMPARE(spy.count(), 1);
	QCOMPARE(spy.first().size(), 2);

	QVariantList args = spy.takeFirst();
	QVariant u = args.at(0);
	QVariant p = args.at(1);
	QVERIFY(u.type() == QVariant::ByteArray);
	QVERIFY(p.type() == QVariant::ByteArray);

	QVERIFY(u.toByteArray() == QByteArray());
	QVERIFY(p.toByteArray() == QByteArray());

	QCOMPARE(this->worker->m_state, Worker::AwaitingAuthenticationState);
	int pos = this->input->pos();

	this->worker->acceptAuthentication();
	QCOMPARE(this->worker->m_state, Worker::AwaitingRequestState);
	QVERIFY(this->input->seek(pos));

	buf = this->input->readAll();
	QCOMPARE(buf.size(), 2);
	QCOMPARE(buf.at(0), '\x01');
	QCOMPARE(buf.at(1), '\x00');
}

void WorkerTest::testAuthByteByByte(void)
{
	QSignalSpy spy(this->worker, SIGNAL(authenticateRequest(QByteArray,QByteArray)));

	this->writeData("\x05\x01\x02");
	QCOMPARE(this->worker->m_state, Worker::AwaitingAuthenticationState);

	QByteArray buf;
	QVERIFY(this->input->seek(3));
	buf = this->input->readAll();
	QCOMPARE(buf.size(), 2);
	QCOMPARE(buf.at(0), '\x05');
	QCOMPARE(buf.at(1), '\x02');

	this->writeData(QByteArray("\x01", 1));
	QCOMPARE(spy.count(), 0);
	QCOMPARE(this->worker->m_expected_length, -1);
	this->writeData(QByteArray("\x00", 1));
	QCOMPARE(spy.count(), 0);
	QCOMPARE(this->worker->m_expected_length, -1);
	this->writeData(QByteArray("\x01", 1));
	QCOMPARE(spy.count(), 0);
	QCOMPARE(this->worker->m_expected_length, 4);
	this->writeData(QByteArray("\x50", 1));
	QCOMPARE(spy.count(), 1);
	QCOMPARE(spy.first().size(), 2);

	QVariantList args = spy.takeFirst();
	QVariant u = args.at(0);
	QVariant p = args.at(1);
	QVERIFY(u.type() == QVariant::ByteArray);
	QVERIFY(p.type() == QVariant::ByteArray);

	QVERIFY(u.toByteArray() == QByteArray());
	QVERIFY(p.toByteArray() == QByteArray("\x50"));

	QCOMPARE(this->worker->m_state, Worker::AwaitingAuthenticationState);
	int pos = this->input->pos();

	this->worker->acceptAuthentication();
	QCOMPARE(this->worker->m_state, Worker::AwaitingRequestState);
	QVERIFY(this->input->seek(pos));

	buf = this->input->readAll();
	QCOMPARE(buf.size(), 2);
	QCOMPARE(buf.at(0), '\x01');
	QCOMPARE(buf.at(1), '\x00');
}
