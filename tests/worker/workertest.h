#ifndef WORKERTEST_H
#define WORKERTEST_H

#include <QtCore/QObject>

class QBuffer;
class Worker;

class WorkerTest : public QObject {
	Q_OBJECT
public:
	WorkerTest(void);

private:
	QBuffer* input;
	Worker* worker;

	void writeData(const QByteArray& data);

protected Q_SLOTS:

private Q_SLOTS:
	void initTestCase(void);
	void cleanupTestCase(void);
	void init(void);
	void cleanup(void);

	void testGreeting(void);
	void testGreetingProtocolFailure1(void);
	void testGreetingProtocolFailure2(void);

	void testTooMuchData(void);

	void testNoAuthSuccess(void);
	void testNoAuthFailure(void);
};

#endif // WORKERTEST_H
