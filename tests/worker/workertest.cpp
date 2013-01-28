#include <QtCore/QBuffer>
#include <QtCore/QCoreApplication>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>
#include "workertest.h"
#include "worker.h"

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

	this->writeData("\x02");
	QVERIFY(!this->worker->m_target);
	QVERIFY(!this->worker->m_connector);
	QCOMPARE(this->worker->m_buf.size(), 0);
	QCOMPARE(this->worker->m_expected_length, -1);
	QCOMPARE(this->worker->m_state, Worker::AwaitingAuthenticationState);

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
}

void WorkerTest::testGreetingProtocolFailure2(void)
{
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
}
