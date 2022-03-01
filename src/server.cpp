#include "server.h"

using namespace std;

Server::Server(QObject *parent) :
    QTcpServer(parent){
#ifdef USB
    m_xpad = new XpadUSBLibrary;
#endif
}

Server::~Server(){
#ifdef USB
    delete m_xpad;
#endif
}

void Server::incomingConnection(qintptr socketID){
    //cout << "Thread number " << socketID << " started" << endl;
    QString message = "> ";
#ifdef PCI
    MainThread *m_thread = new MainThread(socketID, message, this);
#elif USB
    MainThread *m_thread = new MainThread(socketID, message, m_xpad, this);
#endif
    connect(m_thread, SIGNAL(finished()), m_thread, SLOT(deleteLater()));
    m_thread->start();
    m_thread->quit();
}


