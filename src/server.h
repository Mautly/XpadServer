#ifndef SERVER_H
#define SERVER_H

#include <QtNetwork/QTcpServer>

#include "mainthread.h"

#ifdef USB
#include <xpadusblibrary.h>
#endif

class Server : public QTcpServer
{
    Q_OBJECT

public:
    Server(QObject *parent = 0);
    ~Server();

protected:
    void incomingConnection(qintptr socketID);

private:
    //MainThread       *m_thread;             //thread of each TCP connection.
#ifdef USB
    XpadUSBLibrary   *m_xpad;
#endif

};

#endif // SERVER_H
