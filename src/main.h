#ifndef MAIN_H
#define MAIN_H

#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QTcpServer>
#include <QThread>
#include <QDir>

#include <sys/stat.h>
#include <iostream>

#include "server.h"
#include "servertype.h"

#define MAX_LINE 1000

Server              m_server;             //!< Server which creates threads for every TCP socket connection.
int                 m_port_number;        //!< Port number used to connect to server.
unsigned short      m_detector_model;     //!< Variable used to set the detector model.

#endif // MAIN_H
