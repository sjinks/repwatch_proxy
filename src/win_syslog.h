#ifndef SYSLOG_H
#define SYSLOG_H

#include <windows.h>

#define LOG_EMERG   EVENTLOG_ERROR_TYPE         ///< Emergency situation - fatal error
#define LOG_ALERT   EVENTLOG_ERROR_TYPE         ///< Alert
#define LOG_CRIT    EVENTLOG_ERROR_TYPE         ///< Critical error
#define LOG_ERR     EVENTLOG_ERROR_TYPE         ///< Error
#define LOG_WARNING EVENTLOG_WARNING_TYPE       ///< Warning
#define LOG_NOTICE  EVENTLOG_INFORMATION_TYPE   ///< Notice
#define LOG_INFO    EVENTLOG_INFORMATION_TYPE   ///< Information
#define LOG_DEBUG   EVENTLOG_INFORMATION_TYPE   ///< Debug message

#define LOG_USER        (1<<3)

#define LOG_PID         0x01    ///< Log the pid with each message
#define LOG_CONS        0x02    ///< Log on the console if errors in sending
#define LOG_ODELAY      0x04    ///< Delay open until first syslog() (default)
#define LOG_NDELAY      0x08    ///< Don't delay open
#define LOG_NOWAIT      0x10    ///< Don't wait for console forks: DEPRECATED
#define LOG_PERROR      0x20    ///< Log to stderr as well

#define LOG_DAEMON (3<<3)

#ifdef __cplusplus
extern "C" {
#endif

void openlog(const char* name, int flags, int);
void closelog(void);
void syslog(int priority, const char* msg, ...);

#ifdef __cplusplus
}
#endif

#endif // SYSLOG_H
