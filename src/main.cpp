#include "main.h"

#include <QCoreApplication>

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (argc >= 2)
        m_port_number = atol(argv[1]);
    else
        m_port_number = 0;

    if (m_port_number > 0){
        if (!m_server.listen(QHostAddress::Any, m_port_number)) {
            cout << "ERROR: Unable to start the server \n" << m_server.errorString().toStdString().c_str();
            exit(0);
        }
    }else{
        if (!m_server.listen()) {
            cout << "ERROR: Unable to start the server \n" << m_server.errorString().toStdString().c_str();
            exit(0);
        }
    }

    QString ip_address;

    // if we did not find one, use IPv4 localhost
    if (ip_address.isEmpty())
        ip_address = QHostAddress(QHostAddress::LocalHost).toString();

    cout << "XPAD Server v3.1.8 " << endl << "The server is running on \n\nIP: " << ip_address.toStdString().c_str() << "\nport: " << m_server.serverPort() << "\n\n";


    QDir dir;
    QString path = QDir::homePath() + "/XPAD_SERVER";
    dir.mkdir(path);
    path = QDir::homePath() + "/XPAD_SERVER/Configuration";
    dir.mkdir(path);
    path = QDir::homePath() + "/XPAD_SERVER/Calibration";
    dir.mkdir(path);
    path = QDir::homePath() + "/XPAD_SERVER/White";
    dir.mkdir(path);

    path = "/opt/imXPAD/tmp";
    dir.setPath(path);
    dir.removeRecursively();
    dir.mkdir(path);
    path = "/opt/imXPAD/tmp_corrected";
    dir.setPath(path);
    dir.removeRecursively();
    dir.mkdir(path);

#ifdef PCI

    m_detector_model = 0;

    QFile file;
    QTextStream in;

    QString fileName = "/opt/imXPAD/XPAD_SERVER/detector_model.txt";
    file.setFileName(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
        in.setDevice(&file);
        while (!in.atEnd()){
            QString line = in.readLine();

            if (line.contains("PCI_S700") && !line.contains("#"))
                m_detector_model = IMXPAD_S700;
            else if (line.contains("PCI_S1400") && !line.contains("#"))
                m_detector_model = IMXPAD_S1400;
            else if (line.contains("PCI_S70") && !line.contains("#"))
                m_detector_model = IMXPAD_S70;
            else if (line.contains("PCI_S140") && !line.contains("#"))
                m_detector_model = IMXPAD_S140;
            else if (line.contains("PCI_S540") && !line.contains("#"))
                m_detector_model = IMXPAD_S540;
        }

        if (m_detector_model == 0) {
            cout << "Detector model doesn't exist" << endl;
            return -1;
        }
        else{
            cout << "Detector model = "  << m_detector_model << endl;
            xpci_init(0,m_detector_model);

            return a.exec();
        }
    }
    else
        cout << "Detector model not specified" << endl;

#endif

	return a.exec();
}
