#include <QtCore/QBuffer>
#include <QtCore/QCoreApplication>
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
	QCOMPARE(spy.first().at(0), QVariant::fromValue(Worker::ProtocolVersionMismatch));
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
	QCOMPARE(spy.first().at(0), QVariant::fromValue(Worker::UnknownError));
}

void WorkerTest::testTooMuchData(void)
{
	QSignalSpy spy(this->worker, SIGNAL(error(Worker::Error)));

	this->writeData("\x05\x01\x01\x01");
	QCOMPARE(this->worker->m_state, Worker::FatalErrorState);
	QCOMPARE(spy.count(), 1);
	QCOMPARE(spy.first().at(0), QVariant::fromValue(Worker::TooMuchData));
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
