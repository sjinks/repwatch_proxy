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
	void testGreetingTooMuchData(void);

	void testNoAuthSuccess(void);
	void testNoAuthFailure(void);
	void testUnsupportedAuthMethod(void);
	void testAuthTooMuchData(void);
	void testAuthBadVersion(void);
	void testRejectAuth(void);
	void testAcceptAuth(void);
	void testZeroLengthAuth(void);
	void testAuthByteByByte(void);
};

#endif // WORKERTEST_H
