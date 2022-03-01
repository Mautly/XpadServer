#include "detector.h"

using namespace std;
#ifdef PCI
Detector::Detector():
    m_detector_model(0),
    m_chip_number(7),
    m_chip_mask (127),
    m_module_number(1)
{
    m_detector_sync_status = 0;
    m_dead_noisy_image = NULL;
    m_dead_noisy_file_flag = false;
    m_geometrical_corrections_file_flag = false;
    m_white_image_file_flag = false;
    this->showTimers(false);
    this->setDebugMode(false);
    m_image = new Image();

#elif USB
Detector::Detector(XpadUSBLibrary *XPAD):
    m_detector_model(0),
    m_chip_number(7),
    m_chip_mask (127),
    m_module_number(1),
    m_xpad(XPAD)
{
    m_detector_sync_status = 0;
    m_dead_noisy_image = NULL;
    m_dead_noisy_file_flag = false;
    m_geometrical_corrections_file_flag = false;
    m_white_image_file_flag = false;
    this->showTimers(false);
    this->setDebugMode(false);
    m_image = new Image();

#endif

    for (int i=0; i<20; i++){
        m_angle[i] = 0;
        m_position_X[i] = 0;
        m_position_Y[i] = i*120;
    }
}

Detector::~Detector(){
    delete[] m_dead_noisy_image;
    delete m_image;
}


void Detector::sendImage(QTcpSocket *tcpSocket){
    int module_number = 10;
    int chip_number = 7;
    const unsigned short line_number = module_number*IMG_LINE;
    const unsigned short column_number = chip_number*IMG_COLUMN;
    Image temporalImage(line_number,column_number,1);
    temporalImage.transferImage(tcpSocket);

}

bool Detector::init(){
    if (this->getDetectorModelFromFile()){
#ifdef PCI
        if (this->askReady()){
            this->getChipNumber();
            m_image->setImageDimensions(IMG_LINE*m_module_number, IMG_COLUMN*m_chip_number, -1);
            this->readDeadNoisyMask();
            return true;
        }else{
            cout << "ERROR: Detector::" <<  __func__ << " ---> AskReady FAILED." << endl;
            return false;
        }
#elif USB
        if (!m_xpad->init(0, m_detector_model)){
            if (this->askReady()){
                this->getModuleNumber();
                this->getChipNumber();
                m_image->setImageDimensions(IMG_LINE*m_module_number, IMG_COLUMN*m_chip_number, -1);
                this->readDeadNoisyMask();
                return true;
            }else{
                cout << "ERROR: Detector::" <<  __func__ << " ---> AskReady FAILED." << endl;
                return false;
            }
        }else
            return false;
#endif
    }else
        return false;
}

unsigned int Detector::getFirmwareID(){
#ifdef PCI
    return getFirmwareId();
#elif USB
    return m_xpad->getFirmwareId();
#endif
}

#ifdef USB
QString Detector::getUSBDeviceList(){
    QString device_list;
    char *list = m_xpad->getXpadUsbDetector();
    device_list.append(list);
    delete[] list;
    return device_list;
}

bool Detector::setUSBDevice(int USBPort){
    int ret = m_xpad->init(USBPort);

    if (!ret)
        return true;
    else
        return false;
}

#endif

bool Detector::setDetectorModel(unsigned short detectorModel){
    if (m_detector_model == 0 || m_detector_model == detectorModel){
        m_detector_model = detectorModel;

        this->getModuleNumber();
        this->getChipNumber();
        m_image->setImageDimensions(IMG_LINE*m_module_number, IMG_COLUMN*m_chip_number, -1);
#ifdef USB
        m_xpad->setDetectorModel(detectorModel);
#endif
        return true;
    }else{
        cout << "ERROR: Detector::" <<  __func__ << " ---> Detector model doesn't exist." << endl;
        return false;
    }
}


int Detector::getDetectorModel(){
    return m_detector_model;
}

unsigned short Detector::getDetectorModelFromFile(){
    QFile file;
    QTextStream in;

    QString file_name = "/opt/imXPAD/XPAD_SERVER/detector_model.txt";
    file.setFileName(file_name);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
        in.setDevice(&file);
        while (!in.atEnd()){
            QString line = in.readLine();
#ifdef PCI
            if (line.contains("PCI_S700") && !line.contains("#")){
                m_detector_model = IMXPAD_S700; m_module_number_from_file = 10;}
            else if (line.contains("PCI_S1400") && !line.contains("#")){
                m_detector_model = IMXPAD_S1400; m_module_number_from_file = 20;}
            else if (line.contains("PCI_S70") && !line.contains("#")){
                m_detector_model = IMXPAD_S70; m_module_number_from_file = 1;}
            else if (line.contains("PCI_S140") && !line.contains("#")){
                m_detector_model = IMXPAD_S140; m_module_number_from_file = 2;}
            else if (line.contains("PCI_S540") && !line.contains("#")){
                m_detector_model = IMXPAD_S540; m_module_number_from_file = 8;}
#elif USB
            if (line.contains("USB_S10") && !line.contains("#")){
                m_detector_model = XPAD_S10;}
            else if (line.contains("USB_C10") && !line.contains("#")){
                m_detector_model = XPAD_C10;}
            else if (line.contains("USB_A10") && !line.contains("#")){
                m_detector_model = XPAD_A10;}
            else if (line.contains("USB_S70") && !line.contains("#")){
                m_detector_model = XPAD_S70;}
            else if (line.contains("USB_C70") && !line.contains("#")){
                m_detector_model = XPAD_C70;}
            else if (line.contains("USB_S140") && !line.contains("#")){
                m_detector_model = XPAD_S140;}
            else if (line.contains("USB_C140") && !line.contains("#")){
                m_detector_model = XPAD_C140;}
            else if (line.contains("USB_S210")  && !line.contains("#")){
                m_detector_model = XPAD_S210;}
            else if (line.contains("USB_S270") && !line.contains("#")){
                m_detector_model = XPAD_S270;}
            else if (line.contains("USB_C270") && !line.contains("#")){
                m_detector_model = XPAD_C270;}
            else if (line.contains("USB_S340") && !line.contains("#")){
                m_detector_model = XPAD_S340;}
#endif
        }

        this->setDetectorModel(m_detector_model);
        file.close();

        if (m_detector_model == 0) {
            cout << "ERROR: Detector::" <<  __func__ << " ---> Detector model doesn't exist." << endl;
            return -1;
        }else
            return m_detector_model;

    }else{
        cout << "ERROR: Detector::" <<  __func__ << " ---> File doesn't exist." << endl;
        cout << "\t" << file_name.toStdString().c_str() << endl;
        return -1;
    }
}

unsigned short Detector::getModuleNumber(){
#ifdef PCI
    m_module_number = xpci_getLastMod(m_current_mask);
#elif USB
    m_module_number = m_xpad->tools->getNbModMask(m_current_mask);
#endif
    return m_module_number;
}

unsigned short Detector::getChipNumber(){
#ifdef PCI
    return 7;
#elif USB
    return m_xpad->getChipNumber();
#endif
}

unsigned int Detector::getModuleMask(){
    return m_current_mask;
}

unsigned int Detector::getChipMask(){
#ifdef PCI
    return 127;
#elif USB
    return m_xpad->getChipMask();
#endif
}

void Detector::setModuleMask(unsigned int moduleMask){
    m_current_mask = moduleMask;
    this->getModuleNumber();
}

QString Detector::getImageSize(){
    int num_rows, num_cols;
    m_image->getImageInfo(num_rows, num_cols);

    QString message = QString::number(num_rows) + " x " + QString::number(num_cols);

    return message;
}

void Detector::adjustImageSize(bool geometricalCorrectionFlag){

    m_image->setImageDimensions(IMG_LINE*m_module_number, IMG_COLUMN*m_chip_number, -1);

    switch(m_detector_model){
    case 1:
    case 2:
    case 3: {
        m_image->setImageDimensions(IMG_LINE, IMG_COLUMN, -1);
        break;
    }
    }

    if(geometricalCorrectionFlag == true){

        int num_cols_no_border, num_rows_no_border;

        switch(m_detector_model){
        case 1:
        case 2:
        case 3: {
            num_cols_no_border = IMG_COLUMN - 2;
            num_rows_no_border = IMG_LINE - 2;
            break;
        }
        default:{
            num_cols_no_border = IMG_COLUMN*m_chip_number + (m_chip_number-1)*3 - 2;
            num_rows_no_border = IMG_LINE - 2;
            break;
        }
        }

        int rows, cols;

        unsigned int pos_X_max = 0;
        unsigned int pos_Y_max = 0;

        for (int module=m_module_number-1; module>=0; module--){
            int y_origin_position;
            int x_origin_position;
            Image temp(num_rows_no_border,num_cols_no_border,-1);
            Image temp_2 = temp.rotateImage(m_angle[module], y_origin_position, x_origin_position);
            temp_2.getImageInfo(rows,cols);

            if ( (m_position_X[module] + cols - x_origin_position) > pos_X_max)
                pos_X_max = m_position_X[module] + cols - x_origin_position;
            if (m_position_Y[module] + rows - y_origin_position > pos_Y_max)
                pos_Y_max = m_position_Y[module] + rows - y_origin_position;

        }

        m_image->setImageDimensions(pos_Y_max, pos_X_max ,-1);
    }
}


void Detector::setDebugMode(bool value){
#ifdef PCI
    xpci_debugMsg(value);
#elif USB
    m_xpad->setDebugMode(value);
#endif
}

void Detector::showTimers(bool value){
    m_show_timers_flag = value;
}

bool Detector::askReady(){
#ifdef PCI
    if (xpci_modAskReady(&m_current_mask) == -1){
        cout << "ERROR: Detector::" <<  __func__ << " ---> AskReady FAILED." << endl;
        printf("Module Mask = 0x%x\n", m_current_mask);
        return false;
    }
#elif USB
    m_current_mask = m_xpad->command->askReady();
#endif

    printf("Module Mask = 0x%x\n", m_current_mask);

    if (this->getModuleNumber()>0){
        return true;
    }
    else{
        cout << "ERROR: Detector::" <<  __func__ << " ---> AskReady FAILED." << endl;
        return false;
    }
}

bool Detector::digitalTest(unsigned short *buff16, unsigned short value, unsigned short mode){
#ifdef PCI
    int ret = xpci_digitalTest(m_current_mask, 7, buff16, value, mode);

    if (ret == 0)
        return true;
    else{
        cout << "ERROR: Detector::" <<  __func__ << " ---> DigitalTest FAILED." << endl;
        return false;
    }
#elif USB
    int ret = m_xpad->command->digitalTest(m_current_mask, value, mode);

    if (ret == 0){
        unsigned short *buff_temp = m_xpad->command->readImage2B(m_current_mask);
        for (int i=0; i<120*560*m_module_number; i++)
            buff16[i] = buff_temp[i];
        delete[] buff_temp;
        return true;
    }else{
        cout << "ERROR: Detector::" <<  __func__ << " ---> DigitalTest FAILED." << endl;
        return false;
    }
#endif
}

int Detector::loadConfigGFromFile(QString filePath){
#ifdef PCI   
    int ret = imxpad_fileUploadConfigG(filePath.toUtf8().data());
#elif USB
    int ret = m_xpad->loadConfigGFromFile(m_current_mask, filePath.toUtf8().data());
#endif

    return ret;
}

bool Detector::loadConfigG(unsigned int reg, unsigned int value){
#ifdef PCI
    int ret = xpci_modLoadConfigG(m_current_mask, m_chip_mask, reg, value);

    if (ret == 0)
        return true;
    else{
        cout << "ERROR: Detector::" <<  __func__ << " ---> Loading global configuration FAILED." << endl;
        return false;
    }
#elif USB
    unsigned short *buff_temp = m_xpad->command->loadConfigG(m_current_mask, (unsigned short)reg, (unsigned short)value, m_chip_mask);

    if (buff_temp != NULL){
        delete[] buff_temp;
        return true;
    }else{
        cout << "ERROR: Detector::" <<  __func__ << " ---> Loading global configuration FAILED." << endl;
        return false;
    }
#endif
}

bool Detector::readConfigG(unsigned int reg, unsigned int *buff){
#ifdef PCI
    int ret = xpci_modReadConfigG(m_current_mask, m_chip_mask, reg, buff);

    if (ret == 0)
        return true;
    else{
        cout << "ERROR: Detector::" <<  __func__ << " ---> Reading global configuration FAILED." << endl;
        return false;
    }
#elif USB
    unsigned short *buff_temp = m_xpad->command->readConfigG(m_current_mask, reg, m_chip_mask);

    if (buff_temp != NULL){
        int z=0;
        for (int i=0; i<m_module_number; i++){
            for (int j=1; j<(m_chip_number+1); j++){
                buff[z] = buff_temp[j+i*8];
                z++;
            }
        }
        delete[] buff_temp;
        return true;
    }else{
        cout << "ERROR: Detector::" <<  __func__ << " ---> Reading global configuration FAILED." << endl;
        return false;
    }
#endif
}

bool Detector::increaseITHL(){
#ifdef PCI
    int ret = imxpad_incrITHL(m_current_mask);
#elif USB
    int ret = m_xpad->calibration->ithlIncrease(m_current_mask);
#endif

    if (ret == 0)
        return true;
    else{
        cout << "ERROR: Detector::" <<  __func__ << " ---> Increasing ITHL FAILED." << endl;
        return false;
    }
}

bool Detector::decreaseITHL(){

#ifdef PCI
    int ret = imxpad_decrITHL(m_current_mask);
#elif USB
    int ret = m_xpad->calibration->ithlDecrease(m_current_mask);
#endif

    if (ret == 0)
        return true;
    else{
        cout << "ERROR: Detector::" <<  __func__ << " ---> Decreasing ITHL FAILED." << endl;
        return false;
    }
}

bool Detector::loadFlatConfigL(unsigned short value){
#ifdef PCI
    int ret = xpci_modLoadFlatConfig(m_current_mask, m_chip_mask, value);
#elif USB
    int ret = m_xpad->command->loadFlatConfigL(m_current_mask, value, m_chip_mask);
#endif
    if (ret == 0)
        return true;
    else{
        cout << "ERROR: Detector::" <<  __func__ << " ---> Loading flat local configuration FAILED." << endl;
        return false;
    }
}

int Detector::loadConfigLFromFile(QString filePath){
#ifdef PCI
    int ret = imxpad_fileUploadDaclMatrix(filePath.toUtf8().data(), m_current_mask);
#elif USB
    int ret = m_xpad->loadConfigLFromFileToSRAM(m_current_mask, filePath.toUtf8().data());
#endif

    return ret;
}

bool Detector::readConfigL(unsigned short *buff16){
#ifdef PCI
    int ret = xpci_getModConfig(m_current_mask, m_chip_number, buff16);

    if (ret == 0)
        return true;
    else{
        cout << "ERROR: Detector::" <<  __func__ << " ---> Reading local configuration FAILED." << endl;
        return false;
    }
#elif USB
    unsigned short *buff_temp = m_xpad->command->readConfigL(m_current_mask);

    if (buff_temp != NULL){
        for (int i=0; i<120*560*m_module_number; i++)
            buff16[i] = buff_temp[i];
        delete[] buff_temp;
        return true;
    }else{
        cout << "ERROR: Detector::" <<  __func__ << " ---> Reading local configuration FAILED." << endl;
        return false;
    }
#endif
}

bool Detector::loadExposureParameters(unsigned int numImages, unsigned int time, unsigned int waitingTimeBetweenImages, unsigned int overflowTime,
                                      unsigned short inputSignals, unsigned short outputSignals, unsigned short acquisitionMode, unsigned short imageFormat,
                                      unsigned int stackImages){
    //unsigned int overflow = 4000;
#ifdef PCI
    int ret = xpci_modExposureParam(m_current_mask, time, waitingTimeBetweenImages, 0, 0, overflowTime, inputSignals, 0, 0, numImages, outputSignals, imageFormat, 1, 0, (unsigned int)acquisitionMode, stackImages, 0);
#elif USB
    int ret = m_xpad->command->exposeParam(m_current_mask, numImages, time, overflowTime, inputSignals, outputSignals, waitingTimeBetweenImages, acquisitionMode, stackImages);
#endif

    this->getModuleNumber();
    this->getChipNumber();

    if (ret == 0)
        return true;
    else{
        cout << "ERROR: Detector::" <<  __func__ << " ---> Loading expose parameters FAILED." << endl;
        return false;
    }
}


#ifdef PCI
int Detector::asyncAcquisition(IMG_TYPE type, unsigned imageNumber, int burstNumber){
    xpci_clearNumberLastAcquiredAsyncImage();
    int ret = xpci_getImgSeqAsync(type, m_current_mask, imageNumber, burstNumber);

    return ret;
}
#elif USB
int Detector::asyncAcquisition(unsigned int burstNumber){    
    int ret = m_xpad->exposeAsync(m_current_mask, burstNumber);

    return ret;
}
#endif

#ifdef PCI
int Detector::syncAcquisition(IMG_TYPE type, unsigned numberOfImages, void **buff){
    int ret = xpci_getImgSeq(type, m_current_mask, m_chip_number, numberOfImages, buff, 0,0,0,0);

    return ret;
}
#elif USB
int Detector::syncAcquisition(unsigned int burstNumber){
    int ret = m_xpad->command->expose(m_current_mask, burstNumber);

    return ret;
}
#endif


int Detector::createWhiteImage(IMG_TYPE imageType, QString fileName){

    int columns, lines;
    QString file_name = QDir::homePath() + "/XPAD_SERVER/White/" + fileName + ".dat";

#ifdef PCI
    unsigned int pixels_number = 120*560*m_module_number;
    unsigned int **fast_ret = new unsigned int*[1];
    fast_ret[0] = new unsigned int[pixels_number];

    int ret = this->syncAcquisition(imageType, 1, (void**)fast_ret);

    if (ret == 0){
        columns = 560;
        lines = 120 * m_module_number;

        Image image(lines, columns, -1);
        for (int i=0; i<pixels_number; i++)
            image.setPixelValue(i,fast_ret[0][i]);
        if(image.saveImageToDatFile(file_name)){
            delete[] fast_ret[0];
            delete[] fast_ret;
            return 0;

        }else{
            delete[] fast_ret[0];
            delete[] fast_ret;
            return -1;
        }

    }else{
        delete[] fast_ret[0];
        delete[] fast_ret;
        return ret;
    }
#elif USB
    unsigned int *fast_ret = new unsigned int[120*560*m_module_number];

    int ret = this->syncAcquisition(999);

    if (ret == 0){
        if(this->getAsyncImagesFromDisk(imageType, 0, 999, fast_ret)){
            columns = 560;
            lines = 120 * m_module_number;

            Image image(lines, columns, -1);
            for (int i=0; i<120*560*m_module_number; i++)
                image.setPixelValue(i,fast_ret[i]);
            if(image.saveImageToDatFile(file_name)){
                this->cleanSSDImages(999,0);
                delete[] fast_ret;
                return 0;
            }
            else{
                delete[] fast_ret;
                return -1;
            }
        }
        else{
            delete[] fast_ret;
            return -1;
        }
    }else{
        delete[] fast_ret;
        return ret;
    }
#endif
}

bool Detector::deleteWhiteImage(QString fileName){
    QString file_name = QDir::homePath() + "/XPAD_SERVER/White/" + fileName + ".dat";
    return QFile::remove(file_name);
}

bool Detector::setWhiteImageFileName(QString fileName){
    QString file_name = QDir::homePath() + "/XPAD_SERVER/White/" + fileName + ".dat";
    if (QFile::exists(file_name)){
        m_white_image_file_name = file_name;
        return true;
    }else{
        m_white_image_file_name = "";
        return false;
    }
}

QString Detector::getWhiteImageFileName(){
    return m_white_image_file_name;
}

QStringList Detector::getWhiteImagesInDir(){
    QString directory = QDir::homePath() + "/XPAD_SERVER/White/";
    QDir white_dir(directory);
    white_dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QStringList file_list = white_dir.entryList();
    return file_list;
}

bool Detector::getAsyncImagesFromDisk(IMG_TYPE imageType, unsigned int imageNumber, unsigned int burstNumber, unsigned int *imageBuffer){
#ifdef PCI
    int ret = -1;
    unsigned int pixels_number = 120*560*m_module_number;
    //unsigned int *fast_ret = new unsigned int[pixels_number];
    unsigned short *fast_ret16 = new unsigned short[pixels_number];
    //cout << __func__ << " " << m_module_number << " " << m_current_mask << endl;

    if(imageType == B4){
        ret = xpci_getAsyncImageFromDisk(imageType, m_current_mask, imageBuffer, imageNumber, burstNumber);
        //for (int index=0; index<pixels_number; index++)
        //    imageBuffer[index] = fast_ret[index];
    }
    else{
        ret = xpci_getAsyncImageFromDisk(imageType, m_current_mask, fast_ret16, imageNumber, burstNumber);
        for (int index=0; index<pixels_number; index++)
            imageBuffer[index] = fast_ret16[index];
    }

    //delete[] fast_ret;
    delete[] fast_ret16;

    if(ret == 0)
        return true;
    else
        return false;
#elif USB
    unsigned short *buffer;
    if(imageType == B4)
        buffer = new unsigned short[120*1120*m_module_number];
    else
        buffer = new unsigned short [120*560*m_module_number];

    m_xpad->command->readImgFromDisk(m_current_mask, imageType, buffer, imageNumber, burstNumber);

    if (buffer != NULL ){
        m_xpad->tools->bufferToIntBuffer(m_current_mask, imageType, buffer, imageBuffer);
        delete[] buffer;
        return true;
    }else{
        cout << "ERROR: Detector::" << __func__ << " ---> Buffer empty" << endl;
        delete[] buffer;
        return false;
    }
#endif
}

int Detector::getImageCounter(){
#ifdef PCI
    return xpci_getNumberLastAcquiredAsyncImage();
#elif USB
    return m_xpad->getImageCounter();
#endif
}

bool Detector::readGeometricalCorrectionParameters(QString fileName){
    QString     data;
    QStringList list;

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        cout << "WARNING: Detector::" <<  __func__ << " ---> fileName doesn't exist." << endl;
        cout << "\t" << fileName.toStdString().c_str() << endl;
        return false;
    }else{
        int i=0;
        while (!file.atEnd()){
            data = file.readLine();

            if (data.contains("M")){
                list = data.split(QRegExp("[\t\n]"), QString::SkipEmptyParts);
                m_position_X[i] = list[1].toDouble();
                m_position_Y[i] = list[2].toDouble();
                m_angle[i] = list[3].toDouble();
                //cout << "Position X = " << m_position_X[i] <<  " Position Y = " << m_position_Y[i] << " Angle = " << m_angle[i] << endl;
            }

            /*if ((m_position_X[i] < 0) || (m_position_Y[i] < 0)){
                cout << "WARNING: Detector::" << __func__ << " ---> Negative positions in geometrical correction file are not allowed." << endl;
                file.close();
                return false;
            }*/

            i++;
        }

        file.close();

        return true;
    }
}


void Detector::readDeadNoisyMask(){
    QString file_name;
    QFile   file;
    QTextStream in;

    //Checking if Geometrical Corrections file exist
    file_name = QDir::homePath() + "/XPAD_SERVER/geometrical_corrections_" + QString::number(m_detector_model) + ".txt";
    if (this->readGeometricalCorrectionParameters(file_name) == 0)
        m_geometrical_corrections_file_flag = true;
    else
        m_geometrical_corrections_file_flag = false;

    //Checking if Mask file exist
    int line_number = m_module_number*IMG_LINE;
    int column_number = m_chip_number*IMG_COLUMN;
    if (m_dead_noisy_image != NULL)
        delete m_dead_noisy_image;
    m_dead_noisy_image = new float[line_number*column_number];

    file_name = QDir::homePath() + "/XPAD_SERVER/Mask_" + QString::number(m_detector_model) + ".dat";

    file.setFileName(file_name);
    m_dead_noisy_file_flag = file.open(QIODevice::ReadOnly | QIODevice::Text);

    if (m_dead_noisy_file_flag){
        cout << "Dead & Noisy flag = ON" << endl;
        in.setDevice(&file);

        while (!in.atEnd()) {
            QString line = in.readAll();
            QStringList data = line.split(QRegExp("[ \t\n]"), QString::SkipEmptyParts);
            for (int i=0; i<data.length(); i++)
                m_dead_noisy_image[i] = data[i].toFloat();
        }
        file.close();
    }
}

void Detector::createDeadNoisyMask(){
    QThread::sleep(2);
#ifdef PCI
    int module_number = xpci_getModNb(m_current_mask);
    unsigned int pixels_number = 120*560*m_module_number;
    unsigned int **ret;
    ret = new unsigned int *[1];
    ret[0] = new unsigned int[pixels_number];

    xpci_modExposureParam(m_current_mask, 10000000, 5000, 0, 0, 4000, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0);
    xpci_getImgSeq(B4, m_current_mask, 7, 1, (void **)ret, 0, 0, 0, 0);
#elif USB
    int module_number = m_xpad->tools->getNbModMask(m_current_mask);
    m_xpad->command->exposeParam(m_current_mask, 1,10000000,4000,0,0,5000,0,0);
    unsigned int *ret = m_xpad->command->getOneImage(m_current_mask);
#endif

    int row = 120*module_number;
    int col = 560;

    Image dark(row,col,1);
    Image white(row,col,1);
    Image white_data(row,col,1);
    Image mask(row,col,1);

    //Noisy Pixels
    for(int i=0; i<row*col; i++)
#ifdef PCI
        dark.setPixelValue(i,ret[0][i]);
#elif USB
        dark.setPixelValue(i,ret[i]);
#endif
    dark.searchNoisyPixels();



    //Dead Pixels & Flat Field
    //QString file_name = QDir::homePath() + "/XPAD_SERVER/White_" + QString::number(m_detector_model) + ".dat";
    QString file_name = m_white_image_file_name;
    if (white_data.readImageFromDatFile(file_name)){
        white = white_data;
        mask = dark;

        white_data = white_data * mask;
        float mean = white_data.mean();

        white.searchDeadPixels(mean);

        //cout << "Mean value = " << mean << endl;

        for (int i=0; i<row*col; i++)
            if (white_data.getPixelValue(i)>0)
                m_dead_noisy_image[i] = (float) mean / white_data.getPixelValue(i);

        file_name = QDir::homePath() + "/XPAD_SERVER/Mask_" + QString::number(m_detector_model) + ".dat";

        QFile file(file_name);
        file.open(QIODevice::WriteOnly);
        QTextStream out(&file);

        for (int j=0; j<row; j++){
            for (int i=0; i<col; i++){
                if (white.getPixelValue(j,i) == -2){
                    out << -2 << " ";
                    m_dead_noisy_image[j*col+i] = -2;
                }
                else if (dark.getPixelValue(j,i) == -3){
                    out << -3 << " ";
                    m_dead_noisy_image[j*col+i] = -3;
                }else
                    out << m_dead_noisy_image[j*col+i] << " ";
            }
            out << "\n";
        }
        file.close();
        m_white_image_file_flag = true;

    }else{

        for (int i=0; i<row*col; i++)
            m_dead_noisy_image[i] = dark.getPixelValue(i);

        file_name = QDir::homePath() + "/XPAD_SERVER/Mask_" + QString::number(m_detector_model) + ".dat";
        dark.saveImageToDatFile(file_name);
        m_white_image_file_flag = false;
    }

    m_geometrical_corrections_file_flag = true;
    m_dead_noisy_file_flag = true;

#ifdef PCI
    delete[] ret[0];
#endif
    delete[] ret;
}

bool Detector::readTemperatureSensor(float* buff){
#ifdef PCI
    int ret = xpci_modReadTempSensor(m_current_mask, buff);

    if (ret == 0)
        return true;
    else
        return false;
#elif USB
    float *buff_temp = m_xpad->command->readTempSensor(m_current_mask);
    if (buff_temp != NULL){
        int z=0;
        for (int i=0; i<m_module_number; i++){
            for (int j=0; j<(m_chip_number); j++){
                buff[z] = buff_temp[j+i*8];
                z++;
            }
        }
        delete[] buff_temp;
        return true;
    }else{
        cout << "ERROR: Detector::" <<  __func__ << " ---> Reading global configuration FAILED." << endl;
        return false;
    }
#endif
}

int Detector::calibrationBEAM(QString filePath, unsigned int exposureTime, unsigned int ITHLMax, unsigned int configuration){
    unsigned int calib_itune=0, calib_imfp=0;



    switch(configuration){
    case 0: calib_itune = 120; calib_imfp = 5;break;
    case 1: calib_itune = 120; calib_imfp = 25;break;
    case 2: calib_itune = 120; calib_imfp = 52;
    }

#ifdef PCI
    int ret = imxpad_calibration_BEAM(m_current_mask, filePath.toUtf8().data(), exposureTime, ITHLMax, calib_itune, calib_imfp);
#elif USB
    int ret = m_xpad->calibration->calibrationBEAM(m_current_mask, filePath.toUtf8().data(), exposureTime, ITHLMax, calib_itune, calib_imfp);
#endif



    return ret;
}

int Detector::calibrationOTNpulse(QString filePath, unsigned int configuration){
    unsigned int iterations, calib_itune=0, calib_imfp=0;



    switch(configuration){
    case 0: iterations = 20; calib_itune = 120; calib_imfp = 5;break;
    case 1: iterations = 20; calib_itune = 120; calib_imfp = 25;break;
    case 2: iterations = 20; calib_itune = 120; calib_imfp = 52;
    }

#ifdef PCI
    int ret = imxpad_calibration_OTN_pulse(m_current_mask, filePath.toUtf8().data(), iterations, calib_itune, calib_imfp);
#elif USB
    int ret = m_xpad->calibration->calibrationOTN_pulse(m_current_mask, filePath.toUtf8().data(), (unsigned short)configuration);
#endif



    return ret;
}

int Detector::calibrationOTN(QString filePath, unsigned int configuration){
    unsigned int iterations, calib_itune=0, calib_imfp=0;

    switch(configuration){
    case 0: iterations = 20; calib_itune = 120; calib_imfp = 5;break;
    case 1: iterations = 20; calib_itune = 120; calib_imfp = 25;break;
    case 2: iterations = 20; calib_itune = 120; calib_imfp = 52;
    }

#ifdef PCI
    int ret = imxpad_calibration_OTN(m_current_mask, filePath.toUtf8().data(), iterations, calib_itune, calib_imfp);
#elif USB
    int ret = m_xpad->calibration->calibrationOTN(m_current_mask, filePath.toUtf8().data(), (unsigned short)configuration);
#endif



    return ret;
}

int Detector::getDetectorStatus(){                                  //0 - Idle, 1 - Acquiring
#ifdef PCI
    int detector_async_status = xpci_asyncReadStatus();
    int detector_lib_status = xpci_getLibStatus();
#elif USB
    int detector_async_status = m_xpad->command->getExposeBusy();
    int detector_lib_status = m_xpad->getLibStatus();
#endif

    if(detector_lib_status>1)
        return detector_lib_status;
    else if (detector_async_status == 1)
        return 1;
    else
        return 0;
}

bool Detector::resetDetector(){
#ifdef PCI
    xpci_cleanSharedMemory();
    int ret = xpci_modRebootNIOS(m_current_mask);
#elif USB
    int ret = m_xpad->command->resetModules();
#endif

    if(!ret)
        return true;
    else
        return false;
}

bool Detector::getAbortStatus(){
    QFile file("abort.dat");
    if (file.exists())
        return true;
    else
        return false;
}

void Detector::abortCurrentProcess(){
    QFile file("abort.dat");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    file.close();
#ifdef PCI
    xpci_modAbortExposure();
#elif USB
    //m_xpad->init(0);
    m_xpad->command->abortExposure();
#endif

}

void Detector::cleanSharedMemory(){
#ifdef PCI
    xpci_cleanSharedMemory();
#endif
}

void Detector::cleanAbortProcess(){
#ifdef PCI
    xpci_clearAbortProcess();
#endif
    QFile file("abort.dat");
    file.remove();
}

void Detector::cleanSSDImages(unsigned int burstNumber, unsigned int imagesNumber){
#ifdef PCI
    xpci_cleanSSDImages(burstNumber, imagesNumber);
#elif USB
    m_xpad->tools->cleanSSDImages(burstNumber, imagesNumber);
#endif
}

bool Detector::setHV(unsigned char DAC_HV){
#ifdef USB
    if (m_xpad->command->setHV(m_current_mask, DAC_HV) == 0)
        return true;
    else
        return false;
#endif
}

bool Detector::setHVParameters(unsigned int hvCutTime, unsigned int hvDeadTime){
#ifdef USB
    if (m_xpad->command->setHV_param(m_current_mask, hvCutTime, hvDeadTime) == 0)
        return true;
    else
        return false;
#endif
}

bool Detector::processXpad(unsigned int *ret, bool flatFieldCorrectionFlag, bool geometricalCorrectionFlag, bool noisyPixelCorrectionFlag,
                           bool deadPixelCorrectionFlag, bool transferFlag, unsigned short burstNumber, unsigned short imageNumber,
                           unsigned short formatFile, QTcpSocket *tcpSocket, QString outputFilePath){

    bool succed = false;
#ifdef PCI
    xpci_setLibStatus(0);
#elif USB
    m_xpad->setLibStatus(0);
#endif

    //cout << "\tDetector ---> " <<__func__ << " ---> Detector Model: " << m_detector_model << endl;
    //cout.flush();

    //QString file_name =QDir::homePath() + "/XPAD_SERVER/White_" + QString::number(m_detector_model) + ".dat";
    /*QFile file(m_white_image_file_name);
    if (file.exists())
        m_white_image_file_flag = true;
    else
        m_white_image_file_flag = false;

    flatFieldCorrectionFlag = flatFieldCorrectionFlag & m_white_image_file_flag;*/

    switch(m_detector_model){
#ifdef PCI
    case IMXPAD_S70: succed = this->processXpadS70(ret, flatFieldCorrectionFlag, geometricalCorrectionFlag, noisyPixelCorrectionFlag,
                                            deadPixelCorrectionFlag, transferFlag, burstNumber,imageNumber, formatFile,
                                            tcpSocket, outputFilePath); break;
    case IMXPAD_S140: succed = this->processXpadS140(ret, flatFieldCorrectionFlag, geometricalCorrectionFlag, noisyPixelCorrectionFlag,
                                             deadPixelCorrectionFlag, transferFlag, burstNumber,imageNumber, formatFile,
                                             tcpSocket, outputFilePath); break;
    case IMXPAD_S540: succed = this->processXpadS540(ret, flatFieldCorrectionFlag, geometricalCorrectionFlag, noisyPixelCorrectionFlag,
                                             deadPixelCorrectionFlag, transferFlag, burstNumber,imageNumber, formatFile,
                                             tcpSocket, outputFilePath); break;
    case IMXPAD_S700: succed = this->processXpadS700(ret, flatFieldCorrectionFlag, geometricalCorrectionFlag, noisyPixelCorrectionFlag,
                                             deadPixelCorrectionFlag, transferFlag, burstNumber,imageNumber, formatFile,
                                             tcpSocket, outputFilePath); break;
    case IMXPAD_S1400: succed = this->processXpadS1400(ret, flatFieldCorrectionFlag, geometricalCorrectionFlag, noisyPixelCorrectionFlag,
                                              deadPixelCorrectionFlag, transferFlag, burstNumber,imageNumber, formatFile,
                                              tcpSocket, outputFilePath); break;
#elif USB
    case XPAD_S10:
    case XPAD_C10:
    case XPAD_A10: succed = this->processXpadS10(ret, flatFieldCorrectionFlag, geometricalCorrectionFlag, noisyPixelCorrectionFlag,
                                          deadPixelCorrectionFlag, transferFlag, burstNumber,imageNumber, formatFile,
                                          tcpSocket, outputFilePath); break;
    case XPAD_S70:
    case XPAD_C70: succed = this->processXpadS70(ret, flatFieldCorrectionFlag, geometricalCorrectionFlag, noisyPixelCorrectionFlag,
                                          deadPixelCorrectionFlag, transferFlag, burstNumber,imageNumber, formatFile,
                                          tcpSocket, outputFilePath); break;
    case XPAD_S140:
    case XPAD_C140: succed = this->processXpadS140(ret, flatFieldCorrectionFlag, geometricalCorrectionFlag, noisyPixelCorrectionFlag,
                                           deadPixelCorrectionFlag, transferFlag, burstNumber,imageNumber, formatFile,
                                           tcpSocket, outputFilePath); break;

    case XPAD_S210: succed = this->processXpadS210(ret, flatFieldCorrectionFlag, geometricalCorrectionFlag, noisyPixelCorrectionFlag,
                                           deadPixelCorrectionFlag, transferFlag, burstNumber,imageNumber, formatFile,
                                           tcpSocket, outputFilePath); break;
    case XPAD_S270:
    case XPAD_C270: succed = this->processXpadS270(ret, flatFieldCorrectionFlag, geometricalCorrectionFlag, noisyPixelCorrectionFlag,
                                           deadPixelCorrectionFlag, transferFlag, burstNumber,imageNumber, formatFile,
                                           tcpSocket, outputFilePath); break;
    case XPAD_S340: succed = this->processXpadS340(ret, flatFieldCorrectionFlag, geometricalCorrectionFlag, noisyPixelCorrectionFlag,
                                           deadPixelCorrectionFlag, transferFlag, burstNumber,imageNumber, formatFile,
                                           tcpSocket, outputFilePath); break;

#endif


    }

    return succed;
}

bool Detector::processXpadS10(unsigned int *ret, bool flatFieldCorrectionFlag, bool geometricalCorrectionFlag, bool noisyPixelCorrectionFlag,
                              bool deadPixelCorrectionFlag, bool transferFlag, unsigned short burstNumber, unsigned short imageNumber,
                              unsigned short formatFile, QTcpSocket *tcpSocket, QString outputFilePath){
    bool return_value = false;

    // Vars
    timeval start, end;
    double tg1, tg2, tg3;

    // Timing
    gettimeofday(&start, NULL);
    tg1 = start.tv_sec + start.tv_usec / 1000000.0;

    //std::cout << __func__ << endl;

    const unsigned short line_number = m_module_number*IMG_LINE;
    const unsigned short column_number = m_chip_number*IMG_COLUMN;

    Image tmp(line_number,column_number,0);

    /**** Dead and Noisy pixel correction - Flat field correction ****/
    tmp.deadNoisyCorrection(ret, m_dead_noisy_image, m_dead_noisy_file_flag, noisyPixelCorrectionFlag, deadPixelCorrectionFlag, flatFieldCorrectionFlag);
    tmp.flipImage(true); //module is inversed physically

    if (geometricalCorrectionFlag)
        m_image->copyImage(0, 0, tmp.getSubImage(1,IMG_COLUMN*3+1,IMG_LINE-2,IMG_COLUMN*4-2));
    else
        m_image->copyImage(0, 0, tmp.getSubImage(0,IMG_COLUMN*3,IMG_LINE-1,IMG_COLUMN*4-1));

    gettimeofday(&end, NULL);
    tg2 = end.tv_sec + end.tv_usec / 1000000.0;
    if (m_show_timers_flag) cout << "\t--->Time process one S10 image [s]: " << tg2 -tg1 << endl;

    if (transferFlag)
        return_value = m_image->transferImage(tcpSocket);
    else
        return_value = m_image->saveImage(burstNumber, imageNumber, formatFile, outputFilePath);

    gettimeofday(&end, NULL);
    tg3 = end.tv_sec + end.tv_usec / 1000000.0;
    if (m_show_timers_flag) cout << "\t\t--->Time transfer or save one S10 image [s]: " << tg3 -tg2 << endl;

    return return_value;
}

bool Detector::processXpadS70(unsigned int *ret, bool flatFieldCorrectionFlag, bool geometricalCorrectionFlag, bool noisyPixelCorrectionFlag,
                              bool deadPixelCorrectionFlag, bool transferFlag, unsigned short burstNumber, unsigned short imageNumber,
                              unsigned short formatFile, QTcpSocket *tcpSocket, QString outputFilePath){
    bool return_value = false;

    // Vars
    timeval start, end;
    double tg1, tg2, tg3;

    // Timing
    gettimeofday(&start, NULL);
    tg1 = start.tv_sec + start.tv_usec / 1000000.0;

    //std::cout << __func__ << endl;

    const unsigned short line_number = m_module_number*IMG_LINE;
    const unsigned short column_number = m_chip_number*IMG_COLUMN;

    Image tmp(line_number,column_number,0);

    //cout << line_number << endl;
    //cout << column_number << endl;

    /**** Dead and Noisy pixel correction - Flat field correction ****/
    tmp.deadNoisyCorrection(ret, m_dead_noisy_image, m_dead_noisy_file_flag, noisyPixelCorrectionFlag, deadPixelCorrectionFlag, flatFieldCorrectionFlag);
    tmp.flipImage(true); //module is inversed physically

    if (geometricalCorrectionFlag){
        Image tmp_module_expanded = tmp.expandDoublePixelsNoBorders(flatFieldCorrectionFlag);
        //Image tmp_module_expanded_rotated = tmp_module_expanded.rotateImage(m_angle[0], y_origin_position, x_origin_position);
        //cout << "After image rotation :: X_origin_position = " <<  x_origin_position << " ; Y_origin_position = " << y_origin_position << endl;

        //*** Geometrical Corrections****
        m_image->copyImage(0, 0, tmp_module_expanded);

    }else
        m_image->copyImage(0, 0, tmp);


    gettimeofday(&end, NULL);
    tg2 = end.tv_sec + end.tv_usec / 1000000.0;
    if (m_show_timers_flag) cout << "\t--->Time process one S70 image [s]: " << tg2 -tg1 << endl;

    if (transferFlag)
        return_value = m_image->transferImage(tcpSocket);
    else
        return_value = m_image->saveImage(burstNumber, imageNumber, formatFile, outputFilePath);

    gettimeofday(&end, NULL);
    tg3 = end.tv_sec + end.tv_usec / 1000000.0;
    if (m_show_timers_flag) cout << "\t\t--->Time transfer or save one S70 image [s]: " << tg3 -tg2 << endl;

    return return_value;
}

bool Detector::processXpadS140(unsigned int *ret, bool flatFieldCorrectionFlag, bool geometricalCorrectionFlag, bool noisyPixelCorrectionFlag,
                               bool deadPixelCorrectionFlag, bool transferFlag, unsigned short burstNumber, unsigned short imageNumber,
                               unsigned short formatFile, QTcpSocket *tcpSocket, QString outputFilePath){
    bool return_value = false;

    // Vars
    timeval start, end;
    double tg1, tg2, tg3;

    // Timing
    gettimeofday(&start, NULL);
    tg1 = start.tv_sec + start.tv_usec / 1000000.0;

    //std::cout << __func__ << endl;

    const unsigned short line_number = m_module_number*IMG_LINE;
    const unsigned short column_number = m_chip_number*IMG_COLUMN;

    Image tmp(line_number,column_number,0);

    /**** Dead and Noisy pixel correction - Flat field correction ****/
    tmp.deadNoisyCorrection(ret, m_dead_noisy_image, m_dead_noisy_file_flag, noisyPixelCorrectionFlag, deadPixelCorrectionFlag, flatFieldCorrectionFlag);

    for (int module=0; module<m_module_number; module++){

        //*** Geometrical Corrections****
        int position_y;

        Image tmp_module = tmp.getSubImage(IMG_LINE*module,0,IMG_LINE*(module+1)-1,column_number-1);

        if(module == 0)
            position_y = 0;
        else{
            if (geometricalCorrectionFlag)
                position_y = IMG_LINE-2;
            else
                position_y = IMG_LINE;
            //tmp_module.flipImage(true); //module is inversed physically, not needed cause fixed in firmware.
        }

        if (geometricalCorrectionFlag)
            m_image->copyImage(position_y, 0, tmp_module.expandDoublePixelsNoBorders(flatFieldCorrectionFlag));
        else
            m_image->copyImage(position_y, 0, tmp_module);
    }

    gettimeofday(&end, NULL);
    tg2 = end.tv_sec + end.tv_usec / 1000000.0;
    if (m_show_timers_flag) cout << "\t--->Time process one S140 image [s]: " << tg2 -tg1 << endl;

    if (transferFlag)
        return_value = m_image->transferImage(tcpSocket);
    else
        return_value = m_image->saveImage(burstNumber, imageNumber, formatFile, outputFilePath);

    gettimeofday(&end, NULL);
    tg3 = end.tv_sec + end.tv_usec / 1000000.0;
    if (m_show_timers_flag) cout << "\t\t--->Time transfer or save one S140 image [s]: " << tg3 -tg2 << endl;

    return return_value;
}

bool Detector::processXpadS210(unsigned int *ret, bool flatFieldCorrectionFlag, bool geometricalCorrectionFlag, bool noisyPixelCorrectionFlag,
                               bool deadPixelCorrectionFlag, bool transferFlag, unsigned short burstNumber, unsigned short imageNumber,
                               unsigned short formatFile, QTcpSocket *tcpSocket, QString outputFilePath){
    bool return_value = false;

    // Vars
    timeval start, end;
    double tg1, tg2, tg3;

    // Timing
    gettimeofday(&start, NULL);
    tg1 = start.tv_sec + start.tv_usec / 1000000.0;

    //std::cout << __func__ << endl;

    const unsigned short line_number = m_module_number*IMG_LINE;
    const unsigned short column_number = m_chip_number*IMG_COLUMN;

    Image tmp(line_number,column_number,0);

    /**** Dead and Noisy pixel correction - Flat field correction ****/
    tmp.deadNoisyCorrection(ret, m_dead_noisy_image, m_dead_noisy_file_flag, noisyPixelCorrectionFlag, deadPixelCorrectionFlag, flatFieldCorrectionFlag);

    if (geometricalCorrectionFlag){
        for (int module=m_module_number-1; module>=0; module--){
            int y_origin_position;
            int x_origin_position;

            Image tmp_module = tmp.getSubImage(IMG_LINE*module,0,IMG_LINE*(module+1)-1,column_number-1);

            //tmp_module.getImageInfo(row,col);
            //cout << row << " " << col << endl;

            Image tmp_module_expanded = tmp_module.expandDoublePixelsNoBorders(flatFieldCorrectionFlag);

            //tmp_module_expanded.getImageInfo(row,col,value);
            //cout << row << " " << col << endl;

            Image tmp_module_expanded_rotated = tmp_module_expanded.rotateImage(m_angle[module], y_origin_position, x_origin_position);

            //tmp_module_expanded.getImageInfo(row,col,value);
            //cout << row << " " << col << endl;

            m_image->copyImage(m_position_Y[module] - y_origin_position, m_position_X[module] - x_origin_position, tmp_module_expanded_rotated);
        }
    }else
        m_image->copyImage(0, 0, tmp);

    gettimeofday(&end, NULL);
    tg2 = end.tv_sec + end.tv_usec / 1000000.0;
    if (m_show_timers_flag) cout << "\t--->Time process one S270 image [s]: " << tg2 -tg1 << endl;

    if (transferFlag)
        return_value = m_image->transferImage(tcpSocket);
    else
        return_value = m_image->saveImage(burstNumber, imageNumber, formatFile, outputFilePath);

    gettimeofday(&end, NULL);
    tg3 = end.tv_sec + end.tv_usec / 1000000.0;
    if (m_show_timers_flag) cout << "\t\t--->Time transfer or save one S270 image [s]: " << tg3 -tg2 << endl;

    return return_value;
}

bool Detector::processXpadS270(unsigned int *ret, bool flatFieldCorrectionFlag, bool geometricalCorrectionFlag, bool noisyPixelCorrectionFlag,
                               bool deadPixelCorrectionFlag, bool transferFlag, unsigned short burstNumber, unsigned short imageNumber,
                               unsigned short formatFile, QTcpSocket *tcpSocket, QString outputFilePath){
    bool return_value = false;

    // Vars
    timeval start, end;
    double tg1, tg2, tg3;

    // Timing
    gettimeofday(&start, NULL);
    tg1 = start.tv_sec + start.tv_usec / 1000000.0;

    //std::cout << __func__ << endl;

    const unsigned short line_number = m_module_number*IMG_LINE;
    const unsigned short column_number = m_chip_number*IMG_COLUMN;

    Image tmp(line_number,column_number,0);

    /**** Dead and Noisy pixel correction - Flat field correction ****/
    tmp.deadNoisyCorrection(ret, m_dead_noisy_image, m_dead_noisy_file_flag, noisyPixelCorrectionFlag, deadPixelCorrectionFlag, flatFieldCorrectionFlag);

    if (geometricalCorrectionFlag){
        for (int module=m_module_number-1; module>=0; module--){
            int y_origin_position;
            int x_origin_position;

            Image tmp_module = tmp.getSubImage(IMG_LINE*module,0,IMG_LINE*(module+1)-1,column_number-1);

            //tmp_module.getImageInfo(row,col);
            //cout << row << " " << col << endl;

            Image tmp_module_expanded = tmp_module.expandDoublePixelsNoBorders(flatFieldCorrectionFlag);

            //tmp_module_expanded.getImageInfo(row,col,value);
            //cout << row << " " << col << endl;

            Image tmp_module_expanded_rotated = tmp_module_expanded.rotateImage(m_angle[module], y_origin_position, x_origin_position);

            //tmp_module_expanded.getImageInfo(row,col,value);
            //cout << row << " " << col << endl;

            m_image->copyImage(m_position_Y[module] - y_origin_position, m_position_X[module] - x_origin_position, tmp_module_expanded_rotated);
        }
    }else
        m_image->copyImage(0, 0, tmp);

    gettimeofday(&end, NULL);
    tg2 = end.tv_sec + end.tv_usec / 1000000.0;
    if (m_show_timers_flag) cout << "\t--->Time process one S270 image [s]: " << tg2 -tg1 << endl;

    if (transferFlag)
        return_value = m_image->transferImage(tcpSocket);
    else
        return_value = m_image->saveImage(burstNumber, imageNumber, formatFile, outputFilePath);

    gettimeofday(&end, NULL);
    tg3 = end.tv_sec + end.tv_usec / 1000000.0;
    if (m_show_timers_flag) cout << "\t\t--->Time transfer or save one S270 image [s]: " << tg3 -tg2 << endl;

    return return_value;
}

bool Detector::processXpadS340(unsigned int *ret, bool flatFieldCorrectionFlag, bool geometricalCorrectionFlag, bool noisyPixelCorrectionFlag,
                               bool deadPixelCorrectionFlag, bool transferFlag, unsigned short burstNumber, unsigned short imageNumber,
                               unsigned short formatFile, QTcpSocket *tcpSocket, QString outputFilePath){
    bool return_value = false;

    // Vars
    timeval start, end;
    double tg1, tg2, tg3;

    // Timing
    gettimeofday(&start, NULL);
    tg1 = start.tv_sec + start.tv_usec / 1000000.0;

    //std::cout << __func__ << endl;

    const unsigned short line_number = m_module_number*IMG_LINE;
    const unsigned short column_number = m_chip_number*IMG_COLUMN;

    Image tmp(line_number,column_number,0);

    /**** Dead and Noisy pixel correction - Flat field correction ****/
    tmp.deadNoisyCorrection(ret, m_dead_noisy_image, m_dead_noisy_file_flag, noisyPixelCorrectionFlag, deadPixelCorrectionFlag, flatFieldCorrectionFlag);

    if (geometricalCorrectionFlag){
        for (int module=m_module_number-1; module>=0; module--){
            int y_origin_position;
            int x_origin_position;

            Image tmp_module = tmp.getSubImage(IMG_LINE*module,0,IMG_LINE*(module+1)-1,column_number-1);

            //tmp_module.getImageInfo(row,col);
            //cout << row << " " << col << endl;

            Image tmp_module_expanded = tmp_module.expandDoublePixelsNoBorders(flatFieldCorrectionFlag);

            //tmp_module_expanded.getImageInfo(row,col,value);
            //cout << row << " " << col << endl;

            Image tmp_module_expanded_rotated = tmp_module_expanded.rotateImage(m_angle[module], y_origin_position, x_origin_position);

            //tmp_module_expanded.getImageInfo(row,col,value);
            //cout << row << " " << col << endl;

            m_image->copyImage(m_position_Y[module] - y_origin_position, m_position_X[module] - x_origin_position, tmp_module_expanded_rotated);
        }
    }else
        m_image->copyImage(0, 0, tmp);

    gettimeofday(&end, NULL);
    tg2 = end.tv_sec + end.tv_usec / 1000000.0;
    if (m_show_timers_flag) cout << "\t--->Time process one S340 image [s]: " << tg2 -tg1 << endl;

    if (transferFlag)
        return_value = m_image->transferImage(tcpSocket);
    else
        return_value = m_image->saveImage(burstNumber, imageNumber, formatFile, outputFilePath);

    gettimeofday(&end, NULL);
    tg3 = end.tv_sec + end.tv_usec / 1000000.0;
    if (m_show_timers_flag) cout << "\t\t--->Time transfer or save one S340 image [s]: " << tg3 -tg2 << endl;

    return return_value;
}

/*
         * Function processXpadS540
         *
         * Process images aquired by the 540. Any correction is applied.
         *
         * See Also
         * <processXpad>
         */
bool Detector::processXpadS540(unsigned int *ret, bool flatFieldCorrectionFlag, bool geometricalCorrectionFlag, bool noisyPixelCorrectionFlag,
                               bool deadPixelCorrectionFlag, bool transferFlag, unsigned short burstNumber, unsigned short imageNumber,
                               unsigned short formatFile, QTcpSocket *tcpSocket, QString outputFilePath){
    bool return_value = false;

    // Vars
    timeval start, end;
    double tg1, tg2, tg3;

    // Timing
    gettimeofday(&start, NULL);
    tg1 = start.tv_sec + start.tv_usec / 1000000.0;

    //std::cout << __func__ << endl;

    const unsigned short line_number = m_module_number*IMG_LINE;
    const unsigned short column_number = m_chip_number*IMG_COLUMN;

    Image tmp(line_number,column_number,0);

    /**** Dead and Noisy pixel correction - Flat field correction ****/
    tmp.deadNoisyCorrection(ret, m_dead_noisy_image, m_dead_noisy_file_flag, noisyPixelCorrectionFlag, deadPixelCorrectionFlag, flatFieldCorrectionFlag);

    if (geometricalCorrectionFlag){
        for (int module=m_module_number-1; module>=0; module--){
            int y_origin_position;
            int x_origin_position;

            Image tmp_module = tmp.getSubImage(IMG_LINE*module,0,IMG_LINE*(module+1)-1,column_number-1);
            tmp_module.flipImage(false);

            //tmp_module.getImageInfo(row,col);
            //cout << row << " " << col << endl;

            Image tmp_module_expanded = tmp_module.expandDoublePixelsNoBorders(flatFieldCorrectionFlag);

            //tmp_module_expanded.getImageInfo(row,col,value);
            //cout << row << " " << col << endl;

            Image tmp_module_expanded_rotated = tmp_module_expanded.rotateImage(m_angle[module], y_origin_position, x_origin_position);

            //tmp_module_expanded.getImageInfo(row,col,value);
            //cout << row << " " << col << endl;

            m_image->copyImage(m_position_Y[module] - y_origin_position, m_position_X[module] - x_origin_position, tmp_module_expanded_rotated);
        }
    }else
        m_image->copyImage(0, 0, tmp);

    gettimeofday(&end, NULL);
    tg2 = end.tv_sec + end.tv_usec / 1000000.0;
    if (m_show_timers_flag) cout << "\t--->Time process one S540 image [s]: " << tg2 -tg1 << endl;

    if (transferFlag)
        return_value = m_image->transferImage(tcpSocket);
    else
        return_value = m_image->saveImage(burstNumber, imageNumber, formatFile, outputFilePath);

    gettimeofday(&end, NULL);
    tg3 = end.tv_sec + end.tv_usec / 1000000.0;
    if (m_show_timers_flag) cout << "\t\t--->Time transfer or save one S540 image [s]: " << tg3 -tg2 << endl;

    return return_value;
}

bool Detector::processXpadS700(unsigned int *ret, bool flatFieldCorrectionFlag, bool geometricalCorrectionFlag, bool noisyPixelCorrectionFlag,
                               bool deadPixelCorrectionFlag, bool transferFlag, unsigned short burstNumber, unsigned short imageNumber,
                               unsigned short formatFile, QTcpSocket *tcpSocket, QString outputFilePath){
    bool return_value = false;

    // Vars
    timeval start, end;
    double tg1, tg2, tg3;

    // Timing
    gettimeofday(&start, NULL);
    tg1 = start.tv_sec + start.tv_usec / 1000000.0;

    //std::cout << __func__ << endl;

    const unsigned short line_number = m_module_number*IMG_LINE;
    const unsigned short column_number = m_chip_number*IMG_COLUMN;

    Image tmp(line_number,column_number,0);

    /**** Dead and Noisy pixel correction - Flat field correction ****/
    tmp.deadNoisyCorrection(ret, m_dead_noisy_image, m_dead_noisy_file_flag, noisyPixelCorrectionFlag, deadPixelCorrectionFlag, flatFieldCorrectionFlag);

    if (geometricalCorrectionFlag){
        for (int module=m_module_number-1; module>=0; module--){
            int y_origin_position;
            int x_origin_position;

            Image tmp_module = tmp.getSubImage(IMG_LINE*module,0,IMG_LINE*(module+1)-1,column_number-1);

            if (module >= 8) //module are inversed physically
                tmp_module.flipImage(true);
            else
                tmp_module.flipImage(false);

            //tmp_module.getImageInfo(row,col);
            //cout << row << " " << col << endl;

            Image tmp_module_expanded = tmp_module.expandDoublePixelsNoBorders(flatFieldCorrectionFlag);

            //tmp_module_expanded.getImageInfo(row,col,value);
            //cout << row << " " << col << endl;

            Image tmp_module_expanded_rotated = tmp_module_expanded.rotateImage(m_angle[module], y_origin_position, x_origin_position);

            //tmp_module_expanded.getImageInfo(row,col,value);
            //cout << row << " " << col << endl;

            m_image->copyImage(m_position_Y[module] - y_origin_position, m_position_X[module] - x_origin_position, tmp_module_expanded_rotated);
        }
    }else
        m_image->copyImage(0, 0, tmp);

    gettimeofday(&end, NULL);
    tg2 = end.tv_sec + end.tv_usec / 1000000.0;
    if (m_show_timers_flag) cout << "\t--->Time process one S700 image [s]: " << tg2 -tg1 << endl;

    if (transferFlag)
        return_value = m_image->transferImage(tcpSocket);
    else
        return_value = m_image->saveImage(burstNumber, imageNumber, formatFile, outputFilePath);

    gettimeofday(&end, NULL);
    tg3 = end.tv_sec + end.tv_usec / 1000000.0;
    if (m_show_timers_flag) cout << "\t\t--->Time transfer or save one S700 image [s]: " << tg3 -tg2 << endl;

    return return_value;
}

bool Detector::processXpadS1400(unsigned int *ret, bool flatFieldCorrectionFlag, bool geometricalCorrectionFlag, bool noisyPixelCorrectionFlag,
                                bool deadPixelCorrectionFlag, bool transferFlag, unsigned short burstNumber, unsigned short imageNumber,
                                unsigned short formatFile, QTcpSocket *tcpSocket, QString outputFilePath){
    bool return_value = false;

    // Vars
    timeval start, end;
    double tg1, tg2, tg3;

    // Timing
    gettimeofday(&start, NULL);
    tg1 = start.tv_sec + start.tv_usec / 1000000.0;

    //std::cout << __func__ << endl;

    const unsigned short line_number = m_module_number*IMG_LINE;
    const unsigned short column_number = m_chip_number*IMG_COLUMN;

    Image tmp(line_number,column_number, 0);

    /**** Dead and Noisy pixel correction - Flat field correction ****/
    tmp.deadNoisyCorrection(ret, m_dead_noisy_image, m_dead_noisy_file_flag, noisyPixelCorrectionFlag, deadPixelCorrectionFlag, flatFieldCorrectionFlag);

    if (geometricalCorrectionFlag){
        for (int module=m_module_number-1; module>=0; module--){
            int y_origin_position;
            int x_origin_position;

            Image tmp_module = tmp.getSubImage(IMG_LINE*module,0,IMG_LINE*(module+1)-1,column_number-1);
            tmp_module.flipImage(false);

            //tmp_module.getImageInfo(row,col,value);
            //cout << row << " " << col << endl;

            Image tmp_module_expanded= tmp_module.expandDoublePixelsNoBorders(flatFieldCorrectionFlag);

            //tmp_module_expanded.getImageInfo(row,col,value);
            //cout << row << " " << col << endl;

            Image tmp_module_expanded_rotated = tmp_module_expanded.rotateImage(m_angle[module], y_origin_position, x_origin_position);

            //tmp_module_expanded.getImageInfo(row,col,value);
            //cout << row << " " << col << endl;

            m_image->copyImage(m_position_Y[module] - y_origin_position, m_position_X[module] - x_origin_position, tmp_module_expanded_rotated);
        }
    }else
        m_image->copyImage(0, 0, tmp);

    gettimeofday(&end, NULL);
    tg2 = end.tv_sec + end.tv_usec / 1000000.0;
    if (m_show_timers_flag) cout << "\t--->Time process one S1400 image [s]: " << tg2 -tg1 << endl;

    if (transferFlag)
        return_value = m_image->transferImage(tcpSocket);
    else
        return_value = m_image->saveImage(burstNumber, imageNumber, formatFile, outputFilePath);

    gettimeofday(&end, NULL);
    tg3 = end.tv_sec + end.tv_usec / 1000000.0;
    if (m_show_timers_flag) cout << "\t\t--->Time transfer or save one S1400 image [s]: " << tg3 -tg2 << endl;

    return return_value;


    /*    //************ Back angle projection

            qreal angleDeg = 7.5;
            qreal angleRad = angleDeg*(3.1416)/180;
            qreal fraction = 0.005;

            for (int j=0;j<line_number;j++)
                for (int i=0;i<column_number;i++)
                    tempBuf[j*column_number+i] = 0;

            int j;
            qreal hypotenuse = IMG_LINE-1;

            qreal cosAngle = qCos(angleRad);

            for(int w=0; w<moduleNumber; w++){
                for(int i=0; i<column_number; i++){
                    while(hypotenuse >= 0){
                        int line = w*IMG_LINE + ((IMG_LINE-1) - qFloor(hypotenuse));
                        j = w*IMG_LINE + ((IMG_LINE-1) - qFloor(hypotenuse*cosAngle));
                        tempBuf[j*column_number+i] += fraction*(ret[z][(line*column_number+i)]);
                        //cout << "Hypotenuse = " << hypotenuse << ", Line = " << line << ", j = " << j <<  ", Value = " << tempBuf[j*column_number+i] <<  ", i = " << i << endl;
                        hypotenuse = hypotenuse - fraction;
                    }
                    hypotenuse = IMG_LINE-1;
                }
            }

            for (int j=0;j<line_number;j++)
                for (int i=0;i<column_number;i++)
                    ret[z][(j*column_number+i)] = qRound(tempBuf[j*column_number+i]);

            delete[] tempBuf;

            //**************************************
        */
}

