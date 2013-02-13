#ifndef FUNCTIONS_H
#define FUNCTIONS_H

class QHostAddress;

bool inSubnet(const QHostAddress& a, const QHostAddress& subnet, int netmask);

#endif // FUNCTIONS_H
