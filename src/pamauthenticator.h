#ifndef PAMAUTHENTICATOR_H
#define PAMAUTHENTICATOR_H

#include <QtCore/QObject>
#include <security/pam_appl.h>

class PAMAuthenticator : public QObject {
	Q_OBJECT
public:
	PAMAuthenticator(const QByteArray& username, const QByteArray& password, const QByteArray& host, QObject* parent = 0);
	virtual ~PAMAuthenticator(void);

	bool authenticate(void);

private:
	pam_handle_t* m_ph;
	QByteArray m_user;
	QByteArray m_pass;
	QByteArray m_host;

	static int conv(int num_msg, const struct pam_message** msg, struct pam_response** resp, void* appdata);
};

#endif // PAMAUTHENTICATOR_H
