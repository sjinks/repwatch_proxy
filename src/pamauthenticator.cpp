#include <string.h>
#include <new>
#include "qt4compat.h"
#include "pamauthenticator.h"

PAMAuthenticator::PAMAuthenticator(const QByteArray& username, const QByteArray& password, const QByteArray& host, QObject* parent)
	: QObject(parent), m_ph(0), m_user(username), m_pass(password), m_host(host)
{
}

PAMAuthenticator::~PAMAuthenticator(void)
{
	if (this->m_ph) {
		int res = pam_close_session(this->m_ph, 0);
		pam_end(this->m_ph, res);
		this->m_ph = 0;
	}
}

bool PAMAuthenticator::authenticate(void)
{
	pam_conv c;
	c.conv        = PAMAuthenticator::conv;
	c.appdata_ptr = this;

	int res = pam_start("repwatchproxy", 0, &c, &this->m_ph);
	if (res == PAM_SUCCESS) {
		res = pam_set_item(this->m_ph, PAM_RUSER, this->m_user.constData());
		if (res != PAM_SUCCESS) {
			goto getout;
		}

		res = pam_set_item(this->m_ph, PAM_RHOST, this->m_host.constData());
		if (res != PAM_SUCCESS) {
			goto getout;
		}

		res = pam_authenticate(this->m_ph, 0);
		if (res != PAM_SUCCESS) {
			goto getout;
		}

		res = pam_acct_mgmt(this->m_ph, 0);
		if (PAM_NEW_AUTHTOK_REQD == res) {
			res = pam_chauthtok(this->m_ph, PAM_CHANGE_EXPIRED_AUTHTOK);
		}

		if (res != PAM_SUCCESS) {
			goto getout;
		}

		res = pam_setcred(this->m_ph, PAM_ESTABLISH_CRED);
		if (res != PAM_SUCCESS) {
			goto getout;
		}

		res = pam_open_session(this->m_ph, 0);
		if (res != PAM_SUCCESS) {
			goto getout;
		}

		return true;

getout:
		qWarning("%s: %s", Q_FUNC_INFO, pam_strerror(this->m_ph, res));
		pam_end(this->m_ph, res);
	}
	else {
		qCritical("PAM initialization failed");
	}

	this->m_ph = 0;
	return false;
}

int PAMAuthenticator::conv(int num_msg, const pam_message** msg, pam_response** resp, void* appdata)
{
	PAMAuthenticator* self = reinterpret_cast<PAMAuthenticator*>(appdata);
	Q_ASSERT(self != 0);

	if (!resp || !msg) {
		return PAM_CONV_ERR;
	}

	pam_response* r = new(std::nothrow) pam_response[num_msg];
	if (r) {
		for (int i=0; i<num_msg; ++i) {
			r[i].resp_retcode = 0;
			r[i].resp         = 0;

			switch (msg[i]->msg_style) {
				case PAM_PROMPT_ECHO_ON:
					r[i].resp = strdup(self->m_user.constData());
					break;

				case PAM_PROMPT_ECHO_OFF:
					r[i].resp = strdup(self->m_pass.constData());
					break;

				default:
					delete[] r;
					return PAM_CONV_ERR;
			}
		}

		*resp = r;
		return PAM_SUCCESS;
	}

	return PAM_CONV_ERR;
}
