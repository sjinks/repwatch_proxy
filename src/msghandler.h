#ifndef MSGHANDLER_H
#define MSGHANDLER_H

#include <QtCore/QtGlobal>
#include <stdio.h>

#ifdef Q_OS_UNIX
#	include <syslog.h>
#else
#	include "win_syslog.h"
#endif

#if QT_VERSION < 0x050000
void messageHandler(QtMsgType type, const char* msg);
#else
void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);
#endif

void sigsegv_handler(int);
void termination_handler(void);
void print_backtrace(FILE* outb);

#endif // MSGHANDLER_H
