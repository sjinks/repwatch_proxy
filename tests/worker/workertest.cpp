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
	delete this->input;
	delete this->worker;
	this->input = 0;
	this->worker = 0;
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

	QSignalSpy spy(this->worker, SIGNAL(connectionClosed()));
	this->input->close();
	QCoreApplication::processEvents();
	QCOMPARE(spy.count(), 1);

}
