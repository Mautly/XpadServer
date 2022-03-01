#include "mainthread.h"

#include <iostream>

using namespace std;


#ifdef PCI
MainThread::MainThread(qintptr socketDescriptor, const QString &message, QObject *parent) :
    QThread(parent), m_socket_descriptor(socketDescriptor), m_text(message){
    m_detector = new Detector();
}
#elif USB
MainThread::MainThread(qintptr socketDescriptor, const QString &message, XpadUSBLibrary *XPAD, QObject *parent) :
    QThread(parent), m_socket_descriptor(socketDescriptor), m_text(message), m_xpad(XPAD){
    m_detector = new Detector(m_xpad);
}
#endif

MainThread::~MainThread(){
     delete m_detector;
}

void MainThread::run(){
    m_quit_flag = 0;
    m_burst_number = m_socket_descriptor;

    m_tcp_socket = new QTcpSocket();

    if (!m_tcp_socket->setSocketDescriptor(m_socket_descriptor)){
        emit error(m_tcp_socket->error());
        return;
    }

    m_tcp_socket->write("> ");
    m_tcp_socket->waitForBytesWritten();

    //Parameter initialization
    m_number_images = 1;
    m_exposure_time = 1000000;
    m_waiting_time_between_images = 15000;
    m_overflow_time = 4000;
    m_input_signal = 0;
    m_output_signal = 0;
    m_geometrical_correction_flag = false;
    m_flat_field_correction_flag = false;
    m_noisy_pixel_correction_flag = true;
    m_dead_pixel_correction_flag =  true;
    m_image_transfer_flag = true;
    m_output_format_file = 1;     //0 - Ascii, 1 - binary,
    m_acquisition_mode = 0;
    m_output_file_path = "/opt/imXPAD/tmp_corrected/";
    m_white_image_file_name = "NOT_SPECIFIED";

    while(m_quit_flag == 0 && m_tcp_socket->state() == QTcpSocket::ConnectedState)
        this->receivingCommands();

    cout << "DAQ disconnected!" << endl;
    m_tcp_socket->disconnectFromHost();
    delete m_tcp_socket;
}

void MainThread::receivingCommands(){

    m_tcp_socket->waitForReadyRead(16);

    if (m_tcp_socket->bytesAvailable() > 0){

        m_buffer.clear();
        m_buffer = m_tcp_socket->readLine();
        QString buffer = m_buffer;

        cout << "> " << m_buffer.data();

        //***********************************************
        //  GetDetectorStatus
        //***********************************************
        if (buffer.contains("GetDetectorStatus", Qt::CaseInsensitive)){
            this->getDetectorStatus();
        }

        //***********************************************
        // AbortCurrentProcess
        //***********************************************
        else if (buffer.contains("AbortCurrentProcess", Qt::CaseInsensitive)){
            this->abortCurrentProcess();
        }

        //***********************************************
        //  Exit
        //***********************************************
        else if (buffer.contains("Exit", Qt::CaseInsensitive)){
            this->exit();
        }

        //***********************************************
        // ResetDetector
        //***********************************************
        else if (buffer.contains("ResetDetector", Qt::CaseInsensitive)){
            this->resetDetector();
        }

        else if (m_detector->getDetectorStatus()){
            QString message = "Detector BUSY.";
            this->sendWarning(message);
        }

        //***********************************************
        //  SetExposureParameters
        //***********************************************
        else if (buffer.contains("SetExposureParameters", Qt::CaseInsensitive)){
            this->setExposureParameters();
        }

        //***********************************************
        //  StartExposure
        //***********************************************
        else if (buffer.contains("StartExposure", Qt::CaseInsensitive)){
            this->startExposure();
        }

        //***********************************************
        //  CreateWhiteImage
        //***********************************************
        else if (buffer.contains("CreateWhiteImage", Qt::CaseInsensitive)){
            this->createWhiteImage();
        }

        //***********************************************
        //  DeleteWhiteImage
        //***********************************************
        else if (buffer.contains("DeleteWhiteImage", Qt::CaseInsensitive)){
            this->deleteWhiteImage();
        }

        //***********************************************
        //  SetWhiteImage
        //***********************************************
        else if (buffer.contains("SetWhiteImage", Qt::CaseInsensitive)){
            this->setWhiteImage();
        }

        //***********************************************
        //  GetWhiteImagesInDir
        //***********************************************
        else if (buffer.contains("GetWhiteImagesInDir", Qt::CaseInsensitive)){
            this->getWhiteImagesInDir();
        }

        //***********************************************
        //  GetUSBDeviceList
        //***********************************************
        else if(buffer.contains("GetUSBDeviceList", Qt::CaseInsensitive)){
            this->getUSBDeviceList();
        }

        //***********************************************
        //  SetUSBDevice
        //***********************************************
        else if(buffer.contains("SetUSBDevice", Qt::CaseInsensitive)){
            this->setUSBDevice();
        }

        //***********************************************
        //  Init
        //***********************************************
        else if (buffer.contains("Init", Qt::CaseInsensitive)){
            this->init();
        }

        //***********************************************
        //  GetFirmwareID
        //***********************************************
        else if (buffer.contains("GetFirmwareID", Qt::CaseInsensitive)){
            this->getFirmwareID();
        }

        //***********************************************
        //  GetDetectorModel
        //***********************************************
        else if (buffer.contains("GetDetectorModel", Qt::CaseInsensitive)){
            this->getDetectorModel();
        }

        //***********************************************
        //  GetDetectorType
        //***********************************************
        else if (buffer.contains("GetDetectorType", Qt::CaseInsensitive)){
            this->getDetectorType();
        }

        //***********************************************
        //  GetModuleNumber
        //***********************************************
        else if (buffer.contains("GetModuleNumber", Qt::CaseInsensitive)){
            this->getModuleNumber();
        }

        //***********************************************
        //  GetChipNumber
        //***********************************************
        else if (buffer.contains("GetChipNumber", Qt::CaseInsensitive)){
            this->getChipNumber();
        }

        //***********************************************
        //  SetModuleMask
        //***********************************************
        else if (buffer.contains("SetModuleMask", Qt::CaseInsensitive)){
            this->setModuleMask();
        }

        //***********************************************
        //  GetModuleMask
        //***********************************************
        else if (buffer.contains("GetModuleMask", Qt::CaseInsensitive)){
            this->getModuleMask();
        }

        //***********************************************
        //  GetChipMask
        //***********************************************
        else if (buffer.contains("GetChipMask", Qt::CaseInsensitive)){
            this->getChipMask();
        }

        //***********************************************
        //  GetImageSize
        //***********************************************
        else if (buffer.contains("GetImageSize", Qt::CaseInsensitive)){
            this->getImageSize();
        }

        //***********************************************
        //  AskReady
        //***********************************************
        else if (buffer.contains("AskReady", Qt::CaseInsensitive)){
            this->askReady();
        }

        //***********************************************
        //  DigitalTest
        //***********************************************
        else if (buffer.contains("DigitalTest", Qt::CaseInsensitive)){
            this->digitalTest();
        }

        //***********************************************
        //  LoadDefaultConfigG
        //***********************************************
        else if (buffer.contains("LoadDefaultConfigG", Qt::CaseInsensitive)){
            this->loadDefaultConfigG();
        }

        //***********************************************
        //  LoadConfigGFromFile
        //***********************************************
        else if (buffer.contains("LoadConfigGFromFile", Qt::CaseInsensitive)){
            this->loadConfigGFromFile();
        }

        //***********************************************
        //  LoadConfigG
        //***********************************************
        else if (buffer.contains("LoadConfigG", Qt::CaseInsensitive)){
            this->loadConfigG();
        }

        //***********************************************
        // ReadConfigG
        //***********************************************
        else if (buffer.contains("ReadConfigG", Qt::CaseInsensitive)){
            this->readConfigG();
        }

        //***********************************************
        //  ITHLIncrease
        //***********************************************
        else if (buffer.contains("ITHLIncrease", Qt::CaseInsensitive)){
            this->ITHLIncrease();
        }

        //***********************************************
        //  ITHLDecrease
        //***********************************************
        else if (buffer.contains("ITHLDecrease", Qt::CaseInsensitive)){
            this->ITHLDecrease();
        }

        //***********************************************
        //  LoadFlatConfigL
        //***********************************************
        else if (buffer.contains("LoadFlatConfigL", Qt::CaseInsensitive)){
            this->loadFlatConfigL();
        }

        //***********************************************
        //  LoadConfigLFromFile
        //***********************************************
        else if (buffer.contains("LoadConfigLFromFile", Qt::CaseInsensitive)){
            this->loadConfigLFromFile();
        }

        //***********************************************
        //  ReadConfigL
        //***********************************************
        else if (buffer.contains("ReadConfigL", Qt::CaseInsensitive)){
            this->readConfigL();
        }

        //***********************************************
        //  CalibrationBEAM
        //***********************************************
        else if (buffer.contains("CalibrationBEAM", Qt::CaseInsensitive)){
            this->calibrationBEAM();
        }

        //***********************************************
        //  CalibrationOTNPulse
        //***********************************************
        else if (buffer.contains("CalibrationOTNPulse", Qt::CaseInsensitive)){
            this->calibrationOTNPulse();
        }

        //***********************************************
        //  CalibrationOTN
        //***********************************************
        else if (buffer.contains("CalibrationOTN", Qt::CaseInsensitive)){
            this->calibrationOTN();
        }

        //***********************************************
        // ResetDetector
        //***********************************************
        else if (buffer.contains("ResetDetector", Qt::CaseInsensitive)){
            this->resetDetector();
        }

        //***********************************************
        // GetBurstNumber
        //***********************************************
        else if (buffer.contains("GetBurstNumber", Qt::CaseInsensitive)){
            this->getBurstNumber();
        }

        //***********************************************
        // AbortCurrentProcess
        //***********************************************
        else if (buffer.contains("AbortCurrentProcess", Qt::CaseInsensitive)){
            this->abortCurrentProcess();
        }

        //***********************************************
        //  Exit
        //***********************************************
        else if (buffer.contains("Exit", Qt::CaseInsensitive)){
            this->exit();
        }

        //***********************************************
        //  SetImageNumber
        //***********************************************
        else if (buffer.contains("SetImageNumber", Qt::CaseInsensitive)){
            this->setImageNumber();
        }

        //***********************************************
        //  GetImageNumber
        //***********************************************
        else if (buffer.contains("GetImageNumber", Qt::CaseInsensitive)){
            this->getImageNumber();
        }

        //***********************************************
        //  SetExposureTime
        //***********************************************
        else if (buffer.contains("SetExposureTime", Qt::CaseInsensitive)){
            this->setExposureTime();
        }

        //***********************************************
        //  GetExposureTime
        //***********************************************
        else if (buffer.contains("GetExposureTime", Qt::CaseInsensitive)){
            this->getExposureTime();
        }

        //***********************************************
        //  SetOverflowTime
        //***********************************************
        else if (buffer.contains("SetOverflowTime", Qt::CaseInsensitive)){
            this->setOverflowTime();
        }

        //***********************************************
        //  GetOverFlowTime
        //***********************************************
        else if (buffer.contains("GetOverflowTime", Qt::CaseInsensitive)){
            this->getOverflowTime();
        }

        //***********************************************
        //  SetWaitingTimeBetweenImages
        //***********************************************
        else if (buffer.contains("SetWaitingTimeBetweenImages", Qt::CaseInsensitive)){
            this->setWaitingTimeBetweenImages();
        }

        //***********************************************
        //  GetWaitingTimeBetweenImages
        //***********************************************
        else if (buffer.contains("GetWaitingTimeBetweenImages", Qt::CaseInsensitive)){
            this->getWaitingTimeBetweenImages();
        }

        //***********************************************
        //  SetGeometricalCorrectionFlag
        //***********************************************
        else if (buffer.contains("SetGeometricalCorrectionFlag", Qt::CaseInsensitive)){
            this->setGeometricalCorrectionFlag();
        }

        //***********************************************
        //  GetGeometricalCorrectionFlag
        //***********************************************
        else if (buffer.contains("GetGeometricalCorrectionFlag", Qt::CaseInsensitive)){
            this->getGeometricalCorrectionFlag();
        }

        //***********************************************
        //  SetFlatFieldCorrectionFlag
        //***********************************************
        else if (buffer.contains("SetFlatFieldCorrectionFlag", Qt::CaseInsensitive)){
            this->setFlatFieldCorrectionFlag();
        }

        //***********************************************
        //  GetFlatFieldCorrectionFlag
        //***********************************************
        else if (buffer.contains("GetFlatFieldCorrectionFlag", Qt::CaseInsensitive)){
            this->getFlatFieldCorrectionFlag();
        }

        //***********************************************
        //  SetNoisyPixelCorrectionFlag
        //***********************************************
        else if (buffer.contains("SetNoisyPixelCorrectionFlag", Qt::CaseInsensitive)){
            this->setNoisyPixelCorrectionFlag();
        }

        //***********************************************
        //  GetNoisyPixelCorrectionFlag
        //***********************************************
        else if (buffer.contains("GetNoisyPixelCorrectionFlag", Qt::CaseInsensitive)){
            this->getNoisyPixelCorrectionFlag();
        }

        //***********************************************
        //  SetDeadPixelCorrectionFlag
        //***********************************************
        else if (buffer.contains("SetDeadPixelCorrectionFlag", Qt::CaseInsensitive)){
            this->setDeadPixelCorrectionFlag();
        }

        //***********************************************
        //  GetDeadPixelCorrectionFlag
        //***********************************************
        else if (buffer.contains("GetDeadPixelCorrectionFlag", Qt::CaseInsensitive)){
            this->getDeadPixelCorrectionFlag();
        }

        //***********************************************
        //  SetImageTransferFlag
        //***********************************************
        else if (buffer.contains("SetImageTransferFlag", Qt::CaseInsensitive)){
            this->setImageTransferFlag();
        }

        //***********************************************
        //  GetImageTransferFlag
        //***********************************************
        else if (buffer.contains("GetImageTransferFlag", Qt::CaseInsensitive)){
            this->getImageTransferFlag();
        }

        //***********************************************
        //  SetAcquisitionMode
        //***********************************************
        else if (buffer.contains("SetAcquisitionMode", Qt::CaseInsensitive)){
            this->setAcquisitionMode();
        }

        //***********************************************
        //  GetAcquisitionMode
        //***********************************************
        else if (buffer.contains("GetAcquisitionMode", Qt::CaseInsensitive)){
            this->getAcquisitionMode();
        }

        //***********************************************
        //  SetOutputFileFormat
        //***********************************************
        else if (buffer.contains("SetOutputFileFormat", Qt::CaseInsensitive)){
            this->setOutputFormatFile();
        }

        //***********************************************
        //  GetOutputFileFormat
        //***********************************************
        else if (buffer.contains("GetOutputFileFormat", Qt::CaseInsensitive)){
            this->getOutputFormatFile();
        }

        //***********************************************
        //  SetOutputFilePath
        //***********************************************
        else if (buffer.contains("SetOutputFilePath", Qt::CaseInsensitive)){
            this->setOutputFilePath();
        }

        //***********************************************
        //  GetOutputFilePath
        //***********************************************
        else if (buffer.contains("GetOutputFilePath", Qt::CaseInsensitive)){
            this->getOutputFilePath();
        }

        //***********************************************
        //  SetInputSignal
        //***********************************************
        else if (buffer.contains("SetInputSignal", Qt::CaseInsensitive)){
            this->setInputSignal();
        }

        //***********************************************
        //  GetInputSignal
        //***********************************************
        else if (buffer.contains("GetInputSignal", Qt::CaseInsensitive)){
            this->getInputSignal();
        }

        //***********************************************
        //  SetOutputSignal
        //***********************************************
        else if (buffer.contains("SetOutputSignal", Qt::CaseInsensitive)){
            this->setOutputSignal();
        }

        //***********************************************
        //  GetOutputSignal
        //***********************************************
        else if (buffer.contains("GetOutputSignal", Qt::CaseInsensitive)){
            this->getOutputSignal();
        }

        //***********************************************
        //  ReadDetectorTemperature
        //***********************************************
        else if (buffer.contains("ReadDetectorTemperature", Qt::CaseInsensitive)){
            this->readDetectorTemperature();
        }

        //***********************************************
        //  SetDebugMode
        //***********************************************
        else if (buffer.contains("SetDebugMode", Qt::CaseInsensitive)){
            this->setDebugMode();
        }

        //***********************************************
        //  ShowTimers
        //***********************************************
        else if (buffer.contains("ShowTimer", Qt::CaseInsensitive)){
            this->showTimers();
        }
        //***********************************************
        //  SetHVValue
        //***********************************************
        else if (buffer.contains("SetHVValue", Qt::CaseInsensitive)){
            this->setHV();
        }
        //***********************************************
        //  SetHVParameters
        //***********************************************
        else if (buffer.contains("SetHVParameters", Qt::CaseInsensitive)){
            this->setHVParameters();
        }
        //***********************************************
        //  CreateDeadNoisyMask
        //***********************************************
        else if (buffer.contains("CreateDeadNoisyMask", Qt::CaseInsensitive)){
            m_detector->createDeadNoisyMask();
            this->sendOK();
        }
        //***********************************************
        //  SendImage
        //***********************************************
        else if (buffer.contains("SendImage", Qt::CaseInsensitive)){
            this->sendImage();
        }
	else if (buffer.length() < 3){
           //this->sendOK(); 
        }
        else{
            this->sendError("Command not recognized.");
            cout << "Command not recognized" << endl;
        }

        m_tcp_socket->write("> ");
        m_tcp_socket->waitForBytesWritten();
    }
}

void MainThread::sendImage(){
    //for (int i=0; i<100; i++)
        m_detector->sendImage(m_tcp_socket);
}

void MainThread::sendOK(){
    m_tcp_socket->write("* 0\n");
    m_tcp_socket->waitForBytesWritten();
}

void MainThread::sendError(QString message){
    m_tcp_socket->write("* -1\n");
    m_tcp_socket->waitForBytesWritten();

    QString formatted_message = "! ERROR: ";
    formatted_message.append(message);
    formatted_message.append("\n");

    m_tcp_socket->write(formatted_message.toStdString().c_str());
    m_tcp_socket->waitForBytesWritten();
}

void MainThread::sendWarning(QString message){
    m_tcp_socket->write("* 1\n");
    m_tcp_socket->waitForBytesWritten();

    QString formatted_message = "# WARNING: ";
    formatted_message.append(message);
    formatted_message.append("\n");

    m_tcp_socket->write(formatted_message.toStdString().c_str());
    m_tcp_socket->waitForBytesWritten();
}

void MainThread::sendMessage(QString message){
    QString formatted_message = "* \"";
    formatted_message.append(message);
    formatted_message.append("\"\n");

    m_tcp_socket->write(formatted_message.toStdString().c_str());
    m_tcp_socket->waitForBytesWritten();
}

void MainThread::sendIntValue(int value){
    QString formatted_message = "* ";
    formatted_message.append(QString::number(value));
    formatted_message.append("\n");

    m_tcp_socket->write(formatted_message.toStdString().c_str());
    m_tcp_socket->waitForBytesWritten();
}

void MainThread::transferEmptyImage(){
    QDataStream out(&m_buffer, QIODevice::WriteOnly);
    out << (quint32) 0 << (quint32) 0 << (quint32) 0;

    m_tcp_socket->write(m_buffer);
    m_tcp_socket->waitForBytesWritten();

    m_tcp_socket->waitForReadyRead();
    m_tcp_socket->readLine();
}

bool MainThread::transferParametersFile(u_int16_t *ret){
    QByteArray buffer;

    int module_number = m_detector->getModuleNumber();
    int chip_number = m_detector->getChipNumber();

    int line_number = module_number*IMG_LINE;
    int column_number = chip_number*IMG_COLUMN;

    //Orderer for ASCII output file
    for (int j=0;j<line_number;j++) {
        for (int i=0;i<column_number;i++){
            buffer.append(QString::number(ret[j*column_number+i]));
            buffer.append(" ");
        }
        buffer.append("\n");
    }

    quint32 dataSize = buffer.size();
    //cout << dataSize << endl;

    m_buffer.clear();
    QDataStream out(&m_buffer, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::LittleEndian);
    out << (quint32) dataSize;
    out << buffer;

    m_tcp_socket->write(m_buffer);
    m_tcp_socket->waitForBytesWritten();

    buffer.clear();

    m_tcp_socket->waitForReadyRead();
    buffer = m_tcp_socket->readAll();

    if(!buffer.isNull())
        return true;
    else
        return false;
}

bool MainThread::receiveParametersFile(QString fileName){
    QString message;

    QByteArray buffer;
    quint32 data_size = 0;
    quint32 bytes_received = 0;

    m_tcp_socket->waitForReadyRead(3000);

    QDataStream in(m_tcp_socket);
    in.setByteOrder(QDataStream::LittleEndian);
    in >> data_size;

    //cout << data_size << endl;

    //quint32 bytes_received_before = 0;
    while(bytes_received < data_size){
        m_tcp_socket->waitForReadyRead(100);
        buffer.append(m_tcp_socket->read((data_size - bytes_received)));
        bytes_received = buffer.size();
        /*if (bytes_received == bytes_received_before){
            return false;
        }*/
        //bytes_received_before = bytes_received;
        cout << "\tReceived " << bytes_received << " out of " << data_size << " Bytes" << endl;
    }

    QString data_size_value = QString::number(data_size) + "\n";
    m_tcp_socket->write(data_size_value.toStdString().c_str());
    m_tcp_socket->waitForBytesWritten();

    if(data_size > 0){
        QFile file(fileName);

        if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
            message = "ERROR: " + fileName + " could not be opened";
            cout << message.toStdString().c_str() << endl;
            return false;
        }else{
            //Written to ASCII file
            QTextStream out(&file);
            out << buffer;
            file.close();
            return true;
        }
    }else{
        message = "ERROR: File received from TCP client was empty";
        cout << message.toStdString().c_str() << endl;
        return false;
    }
}

unsigned int MainThread::getRegNumberFromRegName(QString registerName){
    if((registerName.contains("AMPTP",Qt::CaseInsensitive)))
        return 31;
    else if((registerName.contains("IMFP",Qt::CaseInsensitive)))
        return 59;
    else if((registerName.contains("IOTA",Qt::CaseInsensitive)))
        return 60;
    else if((registerName.contains("IPRE",Qt::CaseInsensitive)))
        return 61;
    else if((registerName.contains("ITHL",Qt::CaseInsensitive)))
        return 62;
    else if((registerName.contains("ITUNE",Qt::CaseInsensitive)))
        return 63;
    else if((registerName.contains("IBUFF",Qt::CaseInsensitive)))
        return 64;
    else
        return 0;
}

void MainThread::getUSBDeviceList(){
#ifdef PCI
    QString message = " ";
    this->sendMessage(message);
#elif USB
    QString message = m_detector->getUSBDeviceList();
    this->sendMessage(message);
#endif
}

void MainThread::setUSBDevice(){
#ifdef PCI
    QString message = "This is a PCI server, not USB";
    this->sendError(message);
#elif USB
    QString data = m_buffer;
    QStringList list = data.split(" ");

    if (m_detector->setUSBDevice(list[1].toInt()))
        this->sendOK();
    else{
        QString message = "Fail to set USB Device to " + list[1];
        this->sendError(message);
    }
#endif
}

void MainThread::init(){
    m_detector->cleanAbortProcess();

    if (!m_detector->getDetectorStatus()){
        if (m_detector->init()){

            m_current_mask = m_detector->getModuleMask();
            if (m_current_mask > 0)
                this->sendOK();
            else{
                QString message = "No active modules were found.";
                this->sendError(message);
            }
        }else{
            QString message = "Ask Ready FAILED.";
            this->sendError(message);
        }
    }else{
        QString message = "Detector BUSY.";
        this->sendError(message);
    }
}

void MainThread::getFirmwareID(){
    unsigned int firmwareID = m_detector->getFirmwareID();

    if(firmwareID>0){
        this->sendMessage(QString::number(firmwareID,16));
    }else{
        QString message = "Getting firmware ID FAILED.";
        this->sendError(message);
    }
}

/*void MainThread::setDetectorModel(){
    QString data = m_buffer;
    QStringList list = data.split(" ");

    if (list.size() > 1){
        QString model = list[1];
        unsigned short model_num;

        if (model.contains("S700"))
            model_num = 104;
        else if (model.contains("S1400"))
            model_num = 105;
        else if (model.contains("S70"))
            model_num = 101;
        else if (model.contains("S140"))
            model_num = 102;
        else if (model.contains("S540"))
            model_num = 103;

        if (model_num != 0 && m_detector->setDetectorModel(model_num)){
            this->sendOK();
        }else{
            QString message = "Detector model differs with the one set on XPAD server.";
            this->sendError(message);
        }
    }else{
        QString message = "Usage: SetDetectorModel XPAD_MODEL";
        this->sendError(message);
    }
}*/

void MainThread::getDetectorModel(){
    QString message;
#ifdef PCI
    switch(m_detector->getDetectorModel()){
    case IMXPAD_S70: message = "XPAD_S70"; break;
    case IMXPAD_S140: message = "XPAD_S140"; break;
    case IMXPAD_S540: message = "XPAD_S540"; break;
    case IMXPAD_S700: message = "XPAD_S700"; break;
    case IMXPAD_S1400: message = "XPAD_S1400"; break;
    default: message = "Detector model not defined";
    }
#elif USB
    switch(m_detector->getDetectorModel()){
    case XPAD_S10: message = "XPAD_S10"; break;
    case XPAD_C10: message = "XPAD_C10"; break;
    case XPAD_A10: message = "XPAD_A10"; break;
    case XPAD_S70: message = "XPAD_S70"; break;
    case XPAD_C70: message = "XPAD_C70"; break;
    case XPAD_S140: message = "XPAD_S140"; break;
    case XPAD_C140: message = "XPAD_C140"; break;
    case XPAD_S210: message = "XPAD_S210"; break;
    case XPAD_S270: message = "XPAD_S270"; break;
    case XPAD_C270: message = "XPAD_C270"; break;
    case XPAD_S340: message = "XPAD_S340"; break;
    default: message = "Detector model not defined";
    }
#endif
    this->sendMessage(message);
}

void MainThread::getDetectorType(){
#ifdef PCI
    QString message = "PCI";
#elif USB
    QString message = "USB";
#endif
    this->sendMessage(message);
}

void MainThread::getModuleNumber(){
    unsigned short moduleNumber = m_detector->getModuleNumber();

    if(moduleNumber>0){
        this->sendIntValue(moduleNumber);
    }else{
        QString message = "Getting number of modules FAILED.";
        this->sendError(message);
    }
}

void MainThread::getChipNumber(){
    unsigned short chipNumber = m_detector->getChipNumber();

    if(chipNumber>0){
        this->sendIntValue(chipNumber);
    }else{
        QString message = "Getting number of chips FAILED.";
        this->sendError(message);
    }
}

void MainThread::setModuleMask(){
    QString data = m_buffer;
    QStringList list = data.split(" ");

    if(list.size()>1){
        m_current_mask = list[1].toUInt();
        m_detector->setModuleMask(m_current_mask);
        m_detector->adjustImageSize(m_geometrical_correction_flag);
        this->sendOK();
    }else{
        QString message =  "Missing parameters. Usage: setModuleMask modulemask[integer value]";
        this->sendError(message);
    }
}

void MainThread::getModuleMask(){
    unsigned int moduleMask = m_detector->getModuleMask();

    if(moduleMask>0){
        this->sendIntValue(moduleMask);
    }else{
        QString message = "Getting module mask FAILED.";
        this->sendError(message);
    }
}

void MainThread::getChipMask(){
    unsigned int chipMask = m_detector->getChipMask();
    QString message;

    if(chipMask>0){
        this->sendIntValue(chipMask);
    }else{
        message = "Getting chip mask FAILED.";
        this->sendError(message);
    }
}

void MainThread::getImageSize(){
    QString image_size = m_detector->getImageSize();
    this->sendMessage(image_size);
}

void MainThread::askReady(){
    QString message = "Ask Ready FAILED.";

    if (!m_detector->getDetectorStatus()){
        if (m_detector->askReady()){

            m_current_mask = m_detector->getModuleMask();

            if(m_current_mask != 0){
                this->sendOK();
            }else{
                message = "No active modules were found.";
                this->sendError(message);
            }
        }else{
            this->sendError(message);
        }
    }else{
        QString message = "Detector BUSY.";
        this->sendError(message);
    }
}

void MainThread::digitalTest(){
    QString message;
    unsigned short value = 100;
    unsigned short int_mode = 0;

    QString data = m_buffer;
    QStringList list = data.split(" ");

    if(list.size()>1){
        QString mode = list[1];


        if(mode.contains("flat", Qt::CaseInsensitive)){
            int_mode = 0;
        }
        else if (mode.contains("gradient", Qt::CaseInsensitive)){
            int_mode = 2;
        }else
            int_mode = 1;
    }else
        int_mode = 1;

    m_detector->sendImage(m_tcp_socket);
    this->sendOK();

    /*unsigned short module_number = m_detector->getModuleNumber();
    unsigned short *ret16 = new unsigned short[120*560*module_number];
    unsigned int *ret = new unsigned int[120*560*module_number];

    m_detector->setLibStatus(4);
    if(m_detector->digitalTest(ret16, value, int_mode)){

        for (int index=0; index<120*560*module_number; index++)
            ret[index] = (unsigned int) ret16[index];

        //Flags: flat_field_corrections_flag, geometrical_corrections_flag,
        //noisyPixelCorrection, deadPixelCorrectionFlag, image_transfer_flag, burst_number, image_number, formatFile
        if (m_detector->processXpad(ret, false, false, false, false, true, 0, 0, m_output_format_file, m_tcp_socket, m_output_file_path))
            this->sendOK();
        else{
            message = "Processing Digital Test image FAILED.";
            this->sendError(message);
        }

    }else{
        message = "Digital Test FAILED.";
        this->transferEmptyImage();
        this->sendError(message);
    }
    m_detector->setLibStatus(0);

    delete[] ret16;
    delete[] ret;*/
}

void MainThread::loadDefaultConfigG(){
    QString message;
    m_detector->setLibStatus(2);
    if (m_detector->loadConfigG(this->getRegNumberFromRegName("AMPTP"), 0))
        if (m_detector->loadConfigG(this->getRegNumberFromRegName("IMFP"), 25))
            if(m_detector->loadConfigG(this->getRegNumberFromRegName("IOTA"), 40))
                if(m_detector->loadConfigG(this->getRegNumberFromRegName("IPRE"), 60))
                    if(m_detector->loadConfigG(this->getRegNumberFromRegName("ITHL"), 32))
                        if(m_detector->loadConfigG(this->getRegNumberFromRegName("IBUFF"), 0))
                            if(m_detector->loadConfigG(this->getRegNumberFromRegName("ITUNE"), 120))
                                this->sendOK();
                            else{
                                message = "Loading ITUNE value FAILED";
                                this->sendError(message);
                            }
                        else{
                            message = "Loading IBUFF value FAILED";
                            this->sendError(message);
                        }
                    else{
                        message = "Loading ITHL value FAILED";
                        this->sendError(message);
                    }
                else{
                    message = "Loading IPRE value FAILED";
                    this->sendError(message);
                }
            else{
                message = "Loading IOTA value FAILED";
                this->sendError(message);
            }
        else{
            message = "Loading IMFP value FAILED";
            this->sendError(message);
        }
    else{
        message = "Loading AMPTP value FAILED";
        this->sendError(message);
    }
    m_detector->setLibStatus(0);
}

void MainThread::loadConfigGFromFile(){
    QString file_name = QDir::homePath() + "/XPAD_SERVER/Configuration/ConfigGlobal.cfg";
    QString message;

    if (this->receiveParametersFile(file_name)){

         m_detector->setLibStatus(2);

        int ret = m_detector->loadConfigGFromFile(file_name);

        if(ret == 0){
            cout << "Transfer from file to Detector - OK" << endl;
            this->sendOK();
        }else if (ret == 1){
            message = "Configuration CORRUPTED due to ABORT.";
            this->sendWarning(message);
        }else{
            cout << "Transfer from file to Detector - FAILED" << endl;

            message = "Loading global configuration from file FAILED.";
            this->sendError(message);
        }

        m_detector->cleanAbortProcess();
        m_detector->setLibStatus(0);

    }
    else{
        message = "Received file from TCP client was empty.";
        this->sendError(message);
    }
}

void MainThread::loadConfigG(){
    QString data = m_buffer;
    QStringList list = data.split(" ");
    QString message;

    if (list.size() > 2){

        QString register_name = list[1];
        unsigned int value = list[2].toInt();
        unsigned int reg = 0;

        int module_number = m_detector->getModuleNumber();
        int chip_number = m_detector->getChipNumber();
        unsigned int *ret = new unsigned int[chip_number*module_number];

        reg = this->getRegNumberFromRegName(register_name);

        if (reg != 0){
            if(m_detector->loadConfigG(reg, value)){

                if(m_detector->readConfigG(reg, ret)){

                    for (int j=0; j<module_number; j++){
                        message.append("Module_" + QString::number(j) + ": ");
                        for (int i=0; i<chip_number; i++)
                            message.append(QString::number(ret[j*7 + i]) + " ");
                    }
                    this->sendMessage(message);
                }else{
                    message = "Reading global configuration after loading global configuration FAILED.";
                    this->sendError(message);
                }

            }else{
                message = "Loading global configuration FAILED.";
                this->sendError(message);
            }
        }else{
            message = "Register does not exist.";
            this->sendError(message);
        }
        delete[] ret;
    }else{
        message = "Parameters missing. Command usage is: LoadConfigG RegisterName Value";
        this->sendError(message);
    }
}

void MainThread::readConfigG(){
    QString data = m_buffer.data();
    QStringList list = data.split(QRegExp(" "), QString::SkipEmptyParts);
    QString message;

    if (list.size() > 1){
        QString register_name = list[1];
        unsigned int reg = 0;

        int module_number = m_detector->getModuleNumber();
        int chip_number = m_detector->getChipNumber();
        unsigned int *ret = new unsigned int[chip_number*module_number];

        reg = this->getRegNumberFromRegName(register_name);

        if (reg !=0){
            if(m_detector->readConfigG(reg, ret)){

                for (int j=0; j<module_number; j++){
                    message.append("Module_" + QString::number(j) + ": ");
                    for (int i=0; i<chip_number; i++)
                        message.append(QString::number(ret[j*7 + i]) + " ");
                    message.append("; ");
                }
                this->sendMessage(message);


            }else{
                message = "Reading global configuration FAILED.";
                this->sendError(message);
            }
        }else{
            message = "Register does not exist.";
            this->sendError(message);
        }
        delete[] ret;
    }else{
        message = "Parameters missing. Command usage is: ReadConfigG RegisterName";
        this->sendError(message);
    }
}

void MainThread::ITHLIncrease(){
    if(m_detector->increaseITHL()){
        this->sendOK();
    }else{
        QString message = "Increasing ITHL FAILED.";
        this->sendError(message);
    }
}

void MainThread::ITHLDecrease(){
    if(m_detector->decreaseITHL()){
        this->sendOK();
    }else{
        QString message = "Decreasing ITHL FAILED.";
        this->sendError(message);
    }
}

void MainThread::loadFlatConfigL(){
    QString data = m_buffer.data();
    QStringList list = data.split(QRegExp(" "), QString::SkipEmptyParts);
    QString message;

    if (list.size() > 1){
        unsigned short value = list[1].toUShort();
        value = value * 8 + 1;

        if(m_detector->loadFlatConfigL(value)){
            this->sendOK();
        }else{
            message = "Loading Flat local configuration FAILED.";
            this->sendError(message);
        }
    }else{
        message = "Parameters missing. Command usage is: LoadFlatConfigL Value[0-63]";
        this->sendError(message);
    }
}

void MainThread::loadConfigLFromFile(){
    QString file_name = QDir::homePath() + "/XPAD_SERVER/Configuration/ConfigLocal.cfl";
    QString message;

    if (this->receiveParametersFile(file_name)){

        m_detector->setLibStatus(2);

        int ret = m_detector->loadConfigLFromFile(file_name);

        if(ret == 0){
            cout << "Transfer from file to Detector - OK" << endl;

            m_detector->createDeadNoisyMask();
            this->sendOK();

        }else if (ret==1){
            message = "Configuration CORRUPTED due to ABORT.";
            this->sendWarning(message);
        }else{
            cout << "Transfer from file to Detector - FAILED" << endl;

            message = "Loading local configuration from file FAILED.";
            this->sendError(message);
        }

        m_detector->cleanAbortProcess();
        m_detector->setLibStatus(0);

    }
    else{
        message = "Received file from TCP client was empty.";
        this->sendError(message);
    }
}

void MainThread::readConfigL(){
    QString message;
    int module_number = m_detector->getModuleNumber();
    int chip_number = m_detector->getChipNumber();

    unsigned short *ret = new unsigned short[120*560*module_number];

    if(m_detector->readConfigL(ret)){

        int line_number = module_number*IMG_LINE;
        int column_number = chip_number*IMG_COLUMN;

        //Orderer for ASCII output file
        m_buffer.clear();
        for (int j=0;j<line_number;j++) {
            for (int i=0;i<column_number;i++){
                //ret[j*column_number+i] >>= 3;
                m_buffer.append(QString::number(ret[j*column_number+i]));
                m_buffer.append(" ");
            }
            m_buffer.append("\n");
        }

        if(this->transferParametersFile(ret)){
            this->sendOK();
        }else{
            message = "Sending local configuration FAILED.";
            this->sendError(message);
        }
    }
    delete[] ret;
}

void MainThread::setExposureParameters(){
    QString message;
    QString data = m_buffer;
    QStringList list = data.split(" ");

    if(list.size() > 11){
        m_number_images =                   list[1].toUInt();
        m_exposure_time =                   list[2].toUInt();
        m_waiting_time_between_images =     list[3].toUInt();
        m_overflow_time =                   list[4].toUInt();
        m_input_signal =                    list[5].toUInt();
        m_output_signal =                   list[6].toUInt();
        m_geometrical_correction_flag =     (bool)  list[7].toShort();
        m_flat_field_correction_flag =      (bool)  list[8].toShort();
        m_image_transfer_flag =             (bool)  list[9].toUShort();
        m_output_format_file =              list[10].toUShort();     //0 - Ascii, 1 - binary,
        m_acquisition_mode =                list[11].toUShort();
        m_stack_images =                    list[12].toUInt();

        if (list.size() >= 14){
            //for(int i=0;i<list[12].size();i++)
            //    printf("0x%x ",list[12].toStdString().c_str()[i]);
            //cout << endl;

            if (list[13].endsWith("\n") || list[13].endsWith("\r"))
                list[13].chop(1);

            m_output_file_path = list[13];

        }else
            m_output_file_path = "/opt/imXPAD/tmp_corrected/";

        m_image_format = 1; //NOT NEEDED ANYMORE

        m_detector->adjustImageSize(m_geometrical_correction_flag);

#ifdef USB
        if (m_waiting_time_between_images < 10000)
            m_waiting_time_between_images = 10000;
#endif

        if(m_detector->loadExposureParameters(m_number_images, m_exposure_time, m_waiting_time_between_images, m_overflow_time, m_input_signal, m_output_signal, m_acquisition_mode, m_image_format, m_stack_images))
            this->sendOK();
        else{
            message = "Setting exposure parameters FAILED.";
            this->sendError(message);
        }
    }
    else if (list.size() == 1){

        if(m_detector->loadExposureParameters(m_number_images, m_exposure_time, m_waiting_time_between_images, m_overflow_time, m_input_signal, m_output_signal, m_acquisition_mode, m_image_format, m_stack_images))
            this->sendOK();
        else{
            message = "Setting exposure parameters FAILED.";
            this->sendError(message);
        }
    }
    else{
        message = "Missing parameters. Usage: SetExposureParameters NumImages ExposureTime[us] WaitingTimeBetweenImages[us] OverflowTime[us] InputSignals OutputSignals GeometricalCorrectionFlag FlatFieldFlag ImageTransferFlag FormatOutputFile AcquisitionMode OutputFilePath";
        this->sendError(message);
    }
}

void MainThread::startExposure(){
    QString message;

    // Vars
    timeval start, end;
    double tg1, tg2, tg3, tg4, tg5;

    // Timing
    gettimeofday(&start, NULL);
    tg1 = start.tv_sec + start.tv_usec / 1000000.0;

    if(m_detector->loadExposureParameters(m_number_images, m_exposure_time, m_waiting_time_between_images, m_overflow_time, m_input_signal, m_output_signal, m_acquisition_mode, m_image_format, m_stack_images)){
        m_detector->cleanAbortProcess();

        gettimeofday(&end, NULL);
        tg2 = end.tv_sec + end.tv_usec / 1000000.0;
        cout << "Time to set exposure conditions [s]: " << tg2 -tg1 << endl;

        IMG_TYPE image_type = (IMG_TYPE) m_detector->getImageType();

#ifdef PCI        
        switch (m_detector->asyncAcquisition(image_type, m_number_images, m_burst_number)){
 #elif USB
        switch(m_detector->asyncAcquisition(m_burst_number)){
#endif
        case 0:{
            if (m_image_transfer_flag){
                gettimeofday(&end, NULL);
                tg3 = end.tv_sec + end.tv_usec / 1000000.0;
                cout << "Time to call asynchronous acquisition [s]: " << tg3 -tg2 << endl;

                this->processImages(image_type);

                gettimeofday(&end, NULL);
                tg4 = end.tv_sec + end.tv_usec / 1000000.0;
                cout << "Total time to process and transfer all images [s]: " << tg4 -tg3 << endl;
            }else{

                gettimeofday(&end, NULL);
                tg3 = end.tv_sec + end.tv_usec / 1000000.0;
                cout << "Time to call asynchronous acquisition [s]: " << tg3 -tg2 << endl;

                if (QDir(m_output_file_path).exists()){
                    m_processing_thread = new ImageProcessingThread(m_detector, image_type, m_burst_number,m_number_images,
                                                                    m_geometrical_correction_flag, m_flat_field_correction_flag,
                                                                    m_noisy_pixel_correction_flag, m_dead_pixel_correction_flag,
                                                                    m_image_transfer_flag, m_output_format_file, m_output_file_path);

                    connect(m_processing_thread, SIGNAL(finished()), m_processing_thread, SLOT(deleteLater()));
                    m_processing_thread->start();

                    this->sendOK();
                    m_processing_thread->wait();
                    m_processing_thread->quit();

                    gettimeofday(&end, NULL);
                    tg4 = end.tv_sec + end.tv_usec / 1000000.0;
                    cout << "Total time to process and transfer all images [s]: " << tg4 -tg3 << endl;

                    //m_burst_number++;


                }else{
                    cout << "ERROR: MainThread::" <<  __func__ << "Path not found ---> " << m_output_file_path.toStdString().c_str() << endl;
                    message = "Output file path DOES NOT exist.";
                    this->sendError(message);
                }
            }
            break;
        }
        case 1:{
            cout << "WARNING: MainThread::" <<  __func__ << " ---> Asynchronous expose was ABORTED." << endl;
            this->transferEmptyImage();
            message = "Image acquisition ABORTED.";
            this->sendWarning(message);
            break;
        }
        case -1:{
            cout << "ERROR: MainThread::" <<  __func__ << " ---> Asynchronous expose FAILED." << endl;
            this->transferEmptyImage();
            message = "Image acquisition FAILED.";
            this->sendError(message);
        }
        }
    }
    else{
        cout << "ERROR: MainThread::" <<  __func__ << " ---> Setting expose parameters FAILED." << endl;
        this->transferEmptyImage();
        message = "Setting expose parameters FAILED.";
        this->sendError(message);
    }

    // Reporting Statistics
    gettimeofday(&end, NULL);
    tg5 = end.tv_sec + end.tv_usec / 1000000.0;
    cout << "Total time for command StartExposure [s]: " << tg5 -tg1 << endl;
}

void MainThread::createWhiteImage(){
    QString message;
    QString data = m_buffer;
    QStringList list = data.split(" ");

    if (list.size() > 1){
        if (list[1].endsWith("\n") || list[1].endsWith("\r"))
            list[1].chop(1);
        if (list[1].contains("."))
            list[1].chop(4);
        m_white_image_file_name = list[1];
    }
    else{
        message = "Missing parameters. Usage: CreateWhiteImage fileName";
        this->sendError(message);
        return;
    }

    if(m_detector->loadExposureParameters(1, m_exposure_time, m_waiting_time_between_images, m_overflow_time, 0, 0, 0, m_image_format, m_stack_images)){

        IMG_TYPE image_type;
        image_type = (IMG_TYPE) m_detector->getImageType();


        switch(m_detector->createWhiteImage(image_type, m_white_image_file_name)){
        case 0: {
            this->sendOK();
            break;
        }
        case 1: {
            cout << "WARNING: MainThread::" <<  __func__ << " ---> Creation of white image was ABORTED." << endl;
            message = "White image acquisition ABORTED.";
            this->sendWarning(message);
            break;
        }
        case -1: {
            cout << "ERROR: MainThread::" <<  __func__ << " ---> Creation of white image FAILED." << endl;
            message = "White image acquisition FAILED.";
            this->sendError(message);
        }
        }
    }else{
        cout << "ERROR: MainThread::" <<  __func__ << " ---> Setting expose parameters FAILED." << endl;
        message = "Setting expose parameters FAILED.";
        this->sendError(message);
    }
}


void MainThread::deleteWhiteImage(){
    QString message;
    QString data = m_buffer;
    QStringList list = data.split(" ");

    if (list.size() > 1){
        if (list[1].endsWith("\n") || list[1].endsWith("\r"))
            list[1].chop(1);
        if (list[1].contains("."))
            list[1].chop(4);
        m_white_image_file_name = list[1];
    }
    else{
        message = "Missing parameters. Usage: DeleteWhiteImage fileName";
        this->sendError(message);
        return;
    }

    if(m_detector->deleteWhiteImage(m_white_image_file_name))
        this->sendOK();
    else{
        message = "File could not been deleted.";
        this->sendError(message);
    }
}

void MainThread::setWhiteImage(){
    QString message;
    QString data = m_buffer;
    QStringList list = data.split(" ");

    if (list.size() > 1){
        if (list[1].endsWith("\n") || list[1].endsWith("\r"))
            list[1].chop(1);
        if (list[1].contains("."))
            list[1].chop(4);
        m_white_image_file_name = list[1];
    }
    else{
        message = "Missing parameters. Usage: SetWhiteImage fileName";
        this->sendError(message);
        return;
    }
    if (m_detector->setWhiteImageFileName(m_white_image_file_name))
        this->sendOK();
    else{
        message = "White image could not been set cause file DOESN'T exist.";
        this->sendWarning(message);
    }
}

void MainThread::getWhiteImagesInDir(){
    QStringList file_list = m_detector->getWhiteImagesInDir();
    QString message;
    if (file_list.length()>0){
        for (int i=0; i<file_list.length(); i++)
            message += file_list[i] + ", ";
        message.chop(2);
        this->sendMessage(message);
    }else{
        this->sendMessage("Empty directory");
        this->sendWarning("There are not white images stored in the server.");
    }
}

void MainThread::calibrationBEAM(){
    QString message;
    QString data = m_buffer;
    QStringList list = data.split(" ");

    if(list.size()>3){

        unsigned int calib_Texp =           list[1].toUInt();
        unsigned int calib_ithlmax =        list[2].toUInt();
        unsigned int BEAM_configuration =    list[3].toUInt();

        QString fpath = QDir::homePath() + "/XPAD_SERVER/Calibration";
        QDir dir(fpath);
        if (dir.exists(fpath))
            dir.removeRecursively();
        //dir.mkdir(fpath);

        m_detector->setLibStatus(3);
        switch(m_detector->calibrationBEAM(fpath, calib_Texp, calib_ithlmax, BEAM_configuration)){
        case 0: this->sendOK(); break;
        case 1: message = "BEAM Calibration ABORTED."; this->sendWarning(message); break;
        case -1: message = "BEAM Calibration FAILED."; this->sendError(message);
        }
        m_detector->setLibStatus(0);

    }else{
        message = "Missing parameters. Usage: CalibrationBEAM ExposureTime[us] ITHLMax BEAMConfiguration";
        this->sendError(message);
    }
}

void MainThread::calibrationOTNPulse(){
    QString message;

    QString data = m_buffer;
    QStringList list = data.split(" ");

    if(list.size()>1){

        unsigned int OTNConfiguration = list[1].toUInt();

        QString fpath = QDir::homePath() + "/XPAD_SERVER/Calibration";
        QDir dir(fpath);
        if (dir.exists(fpath))
            dir.removeRecursively();
        //dir.mkdir(fpath);

        m_detector->setLibStatus(3);
        switch(m_detector->calibrationOTNpulse(fpath,OTNConfiguration)){
        case 0: m_detector->createDeadNoisyMask(); this->sendOK(); break;
        case 1: message = "Over-The-Noise calibration with pulse ABORTED."; this->sendWarning(message); break;
        case -1: message = "Over-The-Noise calibration with pulse FAILED."; this->sendError(message);
        }
        m_detector->setLibStatus(0);

    }else{
        message = "Missing parameters. Usage: CalibrationOTNPulse OTNConfiguration";
        this->sendError(message);
    }
}

void MainThread::calibrationOTN(){
    QString message;
    QString data = m_buffer;
    QStringList list = data.split(" ");

    if(list.size()>1){

        unsigned int OTNConfiguration = list[1].toUInt();

        QString fpath = QDir::homePath() + "/XPAD_SERVER/Calibration";
        QDir dir(fpath);
        if (dir.exists(fpath))
            dir.removeRecursively();
        //dir.mkdir(fpath);

        m_detector->setLibStatus(3);
        switch(m_detector->calibrationOTN(fpath,OTNConfiguration)){
        case 0: m_detector->createDeadNoisyMask(); this->sendOK(); break;
        case 1: message = "Over-The-Noise calibration ABORTED."; this->sendWarning(message); break;
        case -1: message = "Over-The-Noise calibration FAILED."; this->sendError(message);
        }
        m_detector->setLibStatus(0);

    }else{
        message = "Missing parameters. Usage: CalibrationOTN OTNConfiguration";
        this->sendError(message);
    }
}

void MainThread::getDetectorStatus(){
    QString message;

    int status = m_detector->getDetectorStatus();
    switch (status){
    case 0: message = "Idle. "; break;
    case 1: message = "Acquiring. "; break;
    case 2: message = "Loading/Saving_calibration. "; break;
    case 3: message = "Calibrating. "; break;
    case 4: message = "Digital_Test. "; break;
    case 5: message = "Resetting";
    }
    if(!message.isEmpty())
        this->sendMessage(message);
    else{
        message = "Getting the detector status FAILED.";
        this->sendError(message);
    }
}

void MainThread::resetDetector(){
    QString message;

    m_detector->setLibStatus(5);
    bool ret = m_detector->resetDetector();
    m_detector->setLibStatus(0);

    if(ret)
        this->sendOK();
    else{
        message = "Getting the detector status FAILED.";
        this->sendError(message);
    }
}

void MainThread::getBurstNumber(){
    this->sendIntValue(m_burst_number);
}


void MainThread::abortCurrentProcess(){
    cout << "ABORT in progress..." << endl;
    m_detector->abortCurrentProcess();
#ifdef PCI
    QThread::msleep(100);
    m_detector->resetDetector();
#endif
}

void MainThread::exit(){
    m_quit_flag = 1;
}

void MainThread::setImageNumber(){
    QString data = m_buffer;
    QStringList list = data.split(" ");

    if(list.size()>1){

        m_number_images = list[1].toUInt();
        this->sendOK();
    }else{
        QString message =  "Missing parameters. Usage: SetImageNumber ImageNumber";
        this->sendError(message);
    }
}

void MainThread::getImageNumber(){
    this->sendIntValue(m_number_images);
}

void MainThread::setExposureTime(){
    QString data = m_buffer;
    QStringList list = data.split(" ");

    if(list.size()>1){
        m_exposure_time = list[1].toUInt();
        this->sendOK();
    }else{
        QString message =  "Missing parameters. Usage: setExposureTime Time[us]";
        this->sendError(message);
    }
}

void MainThread::getExposureTime(){
    this->sendIntValue(m_exposure_time);
}

void MainThread::setOverflowTime(){
    QString data = m_buffer;
    QStringList list = data.split(" ");

    if(list.size()>1){

        m_overflow_time = list[1].toUInt();
        this->sendOK();
    }else{
        QString message =  "Missing parameters. Usage: setOverflowTime Time[us]";
        this->sendError(message);
    }
}

void MainThread::getOverflowTime(){
    this->sendIntValue(m_overflow_time);
}

void MainThread::setWaitingTimeBetweenImages(){
    QString data = m_buffer;
    QStringList list = data.split(" ");

    if(list.size()>1){

        m_waiting_time_between_images = list[1].toUInt();
        this->sendOK();
    }else{
        QString message =  "Missing parameters. Usage: SetWaitingTimeBetweenImages Time[us]";
        this->sendError(message);
    }
}

void MainThread::getWaitingTimeBetweenImages(){
    this->sendIntValue(m_waiting_time_between_images);
}

void MainThread::setGeometricalCorrectionFlag(){
    QString message;
    QString data = m_buffer;
    QStringList list = data.split(" ");

    if(list.size()>1){

        QString value = list[1];
        if (value.contains("true", Qt::CaseInsensitive))
            m_geometrical_correction_flag = true;
        else if (value.contains("false", Qt::CaseInsensitive))
            m_geometrical_correction_flag = false;
        else{
            message = "Parameter not recognized";
            this->sendError(message);
            return;
        }

        m_detector->adjustImageSize(m_geometrical_correction_flag);

        this->sendOK();
    }else{
        message =  "Missing parameters. Usage: SetGeometricalCorrectionFlag <true or false>";
        this->sendError(message);
    }
}

void MainThread::getGeometricalCorrectionFlag(){
    QString message;

    if(m_geometrical_correction_flag)
        message = "true";
    else
        message = "false";

    this->sendMessage(message);
}

void MainThread::setFlatFieldCorrectionFlag(){
    QString message;
    QString data = m_buffer;
    QStringList list = data.split(" ");

    if(list.size()>1){

        QString value = list[1];
        if (value.contains("true", Qt::CaseInsensitive))
            m_flat_field_correction_flag = true;
        else if (value.contains("false", Qt::CaseInsensitive))
            m_flat_field_correction_flag = false;
        else{
            message = "Parameter not recognized";
            this->sendError(message);
            return;
        }
        this->sendOK();
    }else{
        message =  "Missing parameters. Usage: SetFlatFieldCorrectionFlag <true or false>";
        this->sendError(message);
    }
}

void MainThread::getFlatFieldCorrectionFlag(){
    QString message;

    if(m_flat_field_correction_flag)
        message = "true";
    else
        message = "false";

    this->sendMessage(message);
}

void MainThread::setNoisyPixelCorrectionFlag(){
    QString message;
    QString data = m_buffer;
    QStringList list = data.split(" ");

    if(list.size()>1){

        QString value = list[1];
        if (value.contains("true", Qt::CaseInsensitive))
            m_noisy_pixel_correction_flag = true;
        else if (value.contains("false", Qt::CaseInsensitive))
            m_noisy_pixel_correction_flag = false;
        else{
            message = "Parameter not recognized";
            this->sendError(message);
            return;
        }

        this->sendOK();
    }else{
        message =  "Missing parameters. Usage: SetNoisyPixelCorrectionFlag <true or false>";
        this->sendError(message);
    }
}

void MainThread::getNoisyPixelCorrectionFlag(){
    QString message;

    if(m_noisy_pixel_correction_flag)
        message = "true";
    else
        message = "false";

    this->sendMessage(message);
}

void MainThread::setDeadPixelCorrectionFlag(){
    QString message;
    QString data = m_buffer;
    QStringList list = data.split(" ");

    if(list.size()>1){

        QString value = list[1];
        if (value.contains("true", Qt::CaseInsensitive))
            m_dead_pixel_correction_flag = true;
        else if (value.contains("false", Qt::CaseInsensitive))
            m_dead_pixel_correction_flag = false;
        else{
            message = "Parameter not recognized";
            this->sendError(message);
            return;
        }
        this->sendOK();
    }else{
        message =  "Missing parameters. Usage: SetDeadPixelCorrectionFlag <true or false>";
        this->sendError(message);
    }
}

void MainThread::getDeadPixelCorrectionFlag(){
    QString message;

    if(m_dead_pixel_correction_flag)
        message = "true";
    else
        message = "false";

    this->sendMessage(message);
}

void MainThread::setImageTransferFlag(){
    QString message;
    QString data = m_buffer;
    QStringList list = data.split(" ");

    if(list.size()>1){

        QString value = list[1];
        if (value.contains("true", Qt::CaseInsensitive))
            m_image_transfer_flag = true;
        else if (value.contains("false", Qt::CaseInsensitive))
            m_image_transfer_flag = false;
        else{
            message = "Parameter not recognized";
            this->sendError(message);
            return;
        }
        this->sendOK();
    }
    else{
        message =  "Missing parameters. Usage: SetImageTransferFlag <true or false>";
        this->sendError(message);
    }
}

void MainThread::getImageTransferFlag(){
    QString message;

    if(m_image_transfer_flag)
        message = "true";
    else
        message = "false";

    this->sendMessage(message);
}

void MainThread::setAcquisitionMode(){
    QString message;
    QString data = m_buffer;
    QStringList list = data.split(" ");

    if(list.size()>1){

        QString value = list[1];
        if (value.contains("Standard", Qt::CaseInsensitive))
            m_acquisition_mode = 0;
        else if (value.contains("Detector_burst", Qt::CaseInsensitive))
            m_acquisition_mode = 1;
        else if (value.contains("Computer_burst", Qt::CaseInsensitive))
            m_acquisition_mode = 2;
        else if (value.contains("Stacking_16_bits", Qt::CaseInsensitive))
            m_acquisition_mode = 3;
        else if (value.contains("Stacking_32_bits", Qt::CaseInsensitive))
            m_acquisition_mode = 4;
        else if (value.contains("Single_bunch_16_bits", Qt::CaseInsensitive))
            m_acquisition_mode = 5;
        else if (value.contains("Single_bunch_32_bits", Qt::CaseInsensitive))
            m_acquisition_mode = 6;
        else{
            message = "Parameter not recognized";
            this->sendError(message);
            return;
        }
        this->sendOK();
    }
    else{
        message =  "Missing parameters. Usage: SetAcquisitionMode <Acquisition_Mode>";
        this->sendError(message);
    }
}

void MainThread::getAcquisitionMode(){
    QString message;
    switch(m_acquisition_mode){
    case 0: message = "Standard"; break;
    case 1: message = "Detector_burst"; break;
    case 2: message = "Computer_burst"; break;
    case 3: message = "Stacking_16_bits"; break;
    case 4: message = "Stacking_32_bits"; break;
    case 5: message = "Single_bunch_16_bits"; break;
    case 6: message = "Single_bunch_32_bits";
    }

    this->sendMessage(message);
}

void MainThread::setOutputFormatFile(){
    QString message;
    QString data = m_buffer;
    QStringList list = data.split(" ");

    if(list.size()>1){

        QString value = list[1];
        if (value.contains("ASCII", Qt::CaseInsensitive))
            m_output_format_file = 0;
        else if (value.contains("Binary", Qt::CaseInsensitive))
            m_output_format_file = 1;
        else{
            message = "Parameter not recognized";
            this->sendError(message);
            return;
        }
        this->sendOK();
    }
    else{
        message =  "Missing parameters. Usage: SetAcquisitionMode <ASCII or Binary>";
        this->sendError(message);
    }
}

void MainThread::getOutputFormatFile(){
    QString message;
    switch(m_output_format_file){
    case 0: message = "ASCII"; break;
    case 1: message = "Binary";
    }

    this->sendMessage(message);
}

void MainThread::setOutputFilePath(){
    QString message;
    QString data = m_buffer;
    QStringList list = data.split(" ");

    if(list.size()>1){

        if (list[1].endsWith("\n"))
            list[1].chop(1);
        if (list[1].endsWith("/"))
            m_output_file_path = list[11];
        else
            m_output_file_path = list[11] + "/";

        if(list[1].contains("default", Qt::CaseInsensitive))
            m_output_file_path = "/opt/imXPAD/tmp_corrected/";

        this->sendOK();
    }else{
        message =  "Missing parameters. Usage: SetOutputFilePath <Path>";
        this->sendError(message);
    }
}

void MainThread::getOutputFilePath(){
    this->sendMessage(m_output_file_path);
}

void MainThread::setInputSignal(){
    QString message;
    QString data = m_buffer;
    QStringList list = data.split(" ");

    if(list.size()>1){

        QString value = list[1];
        if (value.contains("Internal", Qt::CaseInsensitive))
            m_input_signal = 0;
        else if (value.contains("External_gate", Qt::CaseInsensitive))
            m_input_signal = 1;
        else if (value.contains("External_trigger_multiple", Qt::CaseInsensitive))
            m_input_signal = 2;
        else if (value.contains("Eternal_trigger_single", Qt::CaseInsensitive))
            m_input_signal = 3;
        else{
            message = "Parameter not recognized";
            this->sendError(message);
            return;
        }
        this->sendOK();
    }else{
        message =  "Missing parameters. Usage: SetInputSignal <Trigger_Mode>";
        this->sendError(message);
    }
}

void MainThread::getInputSignal(){
    QString message;
    switch(m_input_signal){
    case 0: message = "Internal"; break;
    case 1: message = "External_gate"; break;
    case 2: message = "External_trigger_multiple"; break;
    case 3: message = "External_trigger_single";
    }

    this->sendMessage(message);
}

void MainThread::setOutputSignal(){
    QString message;
    QString data = m_buffer;
    QStringList list = data.split(" ");

    if(list.size()>1){

        QString value = list[1];
        if (value.contains("Exposure_busy", Qt::CaseInsensitive))
            m_output_signal = 0;
        else if (value.contains("Shutter_busy", Qt::CaseInsensitive))
            m_output_signal = 1;
        else if (value.contains("Busy_update_overflow", Qt::CaseInsensitive))
            m_output_signal = 2;
        else if (value.contains("Pixel_counter_enabled", Qt::CaseInsensitive))
            m_output_signal = 3;
        else if (value.contains("External_gate", Qt::CaseInsensitive))
            m_output_signal = 4;
        else if (value.contains("Pixel_counter_enabled", Qt::CaseInsensitive))
            m_output_signal = 5;
        else if (value.contains("Exposure_read_one", Qt::CaseInsensitive))
            m_output_signal = 6;
        else if (value.contains("Data_transfer", Qt::CaseInsensitive))
            m_output_signal = 7;
        else if (value.contains("RAM_ready_image_busy", Qt::CaseInsensitive))
            m_output_signal = 8;
        else if (value.contains("XPAD_to_Local-DDR", Qt::CaseInsensitive))
            m_output_signal = 9;
        else if (value.contains("Local-DDR_to_PC", Qt::CaseInsensitive))
            m_output_signal = 10;
        else{
            message = "Parameter not recognized";
            this->sendError(message);
            return;
        }
        this->sendOK();
    }else{
        message =  "Missing parameters. Usage: SetOutputSignal <Output_Signal>";
        this->sendError(message);
    }
}

void MainThread::getOutputSignal(){
    QString message;
    switch(m_output_signal){
    case 0: message = "Exposure Busy"; break;
    case 1: message = "Shutter Busy"; break;
    case 2: message = "Busy Update Overflow"; break;
    case 3: message = "Pixel Counter Enabled"; break;
    case 4: message = "External Gate"; break;
    case 5: message = "Pixel Counter Enabled"; break;
    case 6: message = "Exposure Read One"; break;
    case 7: message = "Data Transfer"; break;
    case 8: message = "RAM Ready Image Busy"; break;
    case 9: message = "XPAD to Local-DDR"; break;
    case 10: message = "Local-DDR to PC"; break;
    }

    this->sendMessage(message);
}

void MainThread::readDetectorTemperature(){
    QString message;

    int module_number = m_detector->getModuleNumber();
    int chip_number = m_detector->getChipNumber();
    float *ret = new float[chip_number*module_number];

    if(m_detector->readTemperatureSensor(ret)){
        //if(m_detector->readConfigG(reg, ret)){

        for (int j=0; j<module_number; j++){
            message.append("Module_" + QString::number(j) + ": ");
            for (int i=0; i<chip_number; i++)
                message.append(QString::number(ret[j*7 + i]) + " ");
            message.append("; ");
        }
        this->sendMessage(message);
    }else{
        message = "Reading temperature FAILED.";
        this->sendError(message);
    }

    delete[] ret;
}

void MainThread::setHV(){
    QString message;
    QString data = m_buffer;
    QStringList list = data.split(" ");

    if(list.size()>1 && list[1].toUInt()>=0 && list[1].toUInt()<256){

        unsigned char DAC_HV = (unsigned char) list[1].toInt();

        if (m_detector->setHV(DAC_HV))
            this->sendOK();
        else{
            message = "HV value was not set in the detector";
            this->sendError(message);
            return;
        }
    }
    else{
        message =  "Missing parameters. Usage: SetHVValue <value [0-255]>";
        this->sendError(message);
    }
}

void MainThread::setHVParameters(){
    QString message;
    QString data = m_buffer;
    QStringList list = data.split(" ");

    if(list.size()>2){

        unsigned int hvCutTime = list[1].toUInt();
        unsigned int hvDeadTime = list[2].toUInt();

        if (m_detector->setHVParameters(hvCutTime, hvDeadTime))
            this->sendOK();
        else{
            message = "HV parameters were not set in the detector";
            this->sendError(message);
            return;
        }
    }
    else{
        message =  "Missing parameters. Usage: SetHVParameters <hvCutTime> <hvDeadTime>";
        this->sendError(message);
    }
}

void MainThread::processImages(IMG_TYPE imageType){
    QString message;

    // Vars
    timeval start, end;
    double tg1, tg2, tg3, mean_get_time = 0, mean_process_time = 0;

    unsigned int last_acquired_imageNumber = 0;

    //cout << "\tMainThread ---> " << __func__ << " Detector Model = " << m_detector->getDetectorModel() << endl;
    int module_number = m_detector->getModuleNumber();
    //cout << "\tMainThread ---> " << __func__ << " Module Number = " << module_number << endl;
    //cout << "\tMainThread ---> " << __func__ << " Number of images = " << m_number_images << endl;

    unsigned int *fast_ret = new unsigned int[120*560*module_number];

    unsigned int image_counter = 0;
    while (image_counter < m_number_images){

        last_acquired_imageNumber = m_detector->getImageCounter();
        //cout << "\tMainThread ---> " <<__func__ << " Last acquired image: " << last_acquired_imageNumber << "\r";

        // Timing
        gettimeofday(&start, NULL);
        tg1 = start.tv_sec + start.tv_usec / 1000000.0;

        //TCP socket is flush during the waiting time.
        m_tcp_socket->flush();

        if (image_counter < last_acquired_imageNumber){
            if (m_detector->getAsyncImagesFromDisk(imageType, image_counter, m_burst_number, fast_ret)){

                gettimeofday(&end, NULL);
                tg2 = end.tv_sec + end.tv_usec / 1000000.0;
                mean_get_time += tg2-tg1;

                //cout << "\tMainThread ---> " << __func__ << " ---> Reading image " << image_counter << "\n";
                fflush(stdout);

                if (!m_detector->processXpad(fast_ret, m_flat_field_correction_flag, m_geometrical_correction_flag, m_noisy_pixel_correction_flag,
                                        m_dead_pixel_correction_flag, m_image_transfer_flag, m_burst_number, image_counter,
                                        m_output_format_file, m_tcp_socket, m_output_file_path)){
                    image_counter = m_number_images;
                    m_quit_flag = 1;
                }

                gettimeofday(&end, NULL);
                tg3 = end.tv_sec + end.tv_usec / 1000000.0;
                mean_process_time += tg3-tg2;

                //cout << "\tMainThread ---> " << __func__ << " ---> Processing image " << image_counter << "\n";
                fflush(stdout);

                if (!m_detector->getAbortStatus()){
                    image_counter++;
                }
                else{
                    if(m_image_transfer_flag){
                        if (image_counter < (m_number_images-1)){
                            this->transferEmptyImage();
                            message = "Image acquisition ABORTED.";
                            this->sendWarning(message);
                            while(m_tcp_socket->flush());
                        }else{
                            this->sendOK();
                            while(m_tcp_socket->flush());
                        }
                    }
                    m_detector->cleanSSDImages(m_burst_number, m_number_images);
                    m_detector->cleanAbortProcess();
                    delete[] fast_ret;
                    return;
                }
            }else{
                if (m_detector->getAbortStatus()){
                    if(m_image_transfer_flag){
                        this->transferEmptyImage();
                        message = "Image acquisition FAILED.";
                        this->sendError(message);
                    }
                    m_detector->cleanSSDImages(m_burst_number, m_number_images);
                    m_detector->cleanAbortProcess();
                    delete[] fast_ret;
                    return;
                }
            }
        }
        QThread::usleep(1);
    }

    if(m_image_transfer_flag){
        this->sendOK();
    }  

    m_detector->cleanSSDImages(m_burst_number, m_number_images);
    delete[] fast_ret;

    // Reporting Statistics
    cout << "Mean time to get one image from disk [s]: " << mean_get_time/m_number_images << endl;
    cout << "Mean time to process and transfer one image [s]: " << mean_process_time/m_number_images << endl;   
}

void MainThread::setDebugMode(){
    QString message;
    QString data = m_buffer;
    QStringList list = data.split(" ");

    if(list.size()>1){

        QString value = list[1];

        if (value.contains("true", Qt::CaseInsensitive))
            m_detector->setDebugMode(true);
        else if (value.contains("false", Qt::CaseInsensitive))
            m_detector->setDebugMode(false);
        else{
            message = "Parameter not recognized";
            this->sendError(message);
            return;
        }
        this->sendOK();
    }else{
        message =  "Missing parameters. Usage: SetDebugMode <true or false>";
        this->sendError(message);
    }
}

void MainThread::showTimers(){
    QString message;
    QString data = m_buffer;
    QStringList list = data.split(" ");

    if(list.size()>1){

        QString value = list[1];
        if (value.contains("true", Qt::CaseInsensitive)){
            m_detector->showTimers(true);
        }
        else if (value.contains("false", Qt::CaseInsensitive)){
            m_detector->showTimers(false);
        }
        else{
            message = "Parameter not recognized";
            this->sendError(message);
            return;
        }
        this->sendOK();
    }else{
        message =  "Missing parameters. Usage: ShowTimers <true or false>";
        this->sendError(message);
    }
}
