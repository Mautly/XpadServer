#ifndef DETECTOR_H
#define DETECTOR_H

#include <QThread>
#include <QBuffer>
#include <iostream>
#include <sys/time.h>

#include "servertype.h"

#ifdef PCI
#include <xpci_interface.h>
#include <xpci_interface_expert.h>
#include <xpci_time.h>
#include <xpci_imxpad.h>
#include <xpci_calib_imxpad.h>
#endif
#ifdef USB
#include <xpadusblibrary.h>
#endif

#include "image.h"


class Detector
{
public:
#ifdef PCI
    Detector();
#elif USB
    Detector(XpadUSBLibrary *XPAD);
#endif
    ~Detector();

    //!< Initialization of the detector, check askReady, get image dimensions and dead and noisy mask.
    bool init();

    //!< Initialization of the detector with a given moduleMask, check askReady, get image dimensions and get and noisy mask.
    //bool init(unsigned int moduleMask);

    //!< Get the ID of detector firmware
    unsigned int getFirmwareID();

    //!< Get the list of USB devices connected to the server.
    QString getUSBDeviceList();

#ifdef USB
    //!< Set the USB Device port
    bool setUSBDevice(int USBPort);
#endif

    //!< Set the detector model to be used.
    bool setDetectorModel(unsigned short detectorModel);

    //!< Get the current detector model being used.
    int getDetectorModel();

    //!< Get the detector model register in the "/XPAD_SERVER/detector_model.txt" file.
    unsigned short getDetectorModelFromFile();

    //!< Get the current number of modules that reply in the detector.
    unsigned short getModuleNumber();

    //!< Get the number of chips for each module in the detector.
    unsigned short getChipNumber();

    //!< Get the mask of the current number of modules.
    unsigned int getModuleMask();

    //!< Get the mask of the number of chips for each module in the detector.
    unsigned int getChipMask();

    //!< Set the mask for the current number of modules.
    void setModuleMask(unsigned int moduleMask);

    //!< Get image size given by the detectorµ.
    QString getImageSize();

    //!< Adjust image size if geometrical correction is activated or not.
    void adjustImageSize(bool geometricalCorrectionFlag);

    //!< Turn on/off the debug mode on the library.
    void setDebugMode(bool value);

    //!< Turn on/off the timers for image processing and transfer.
    void showTimers(bool value);

    //!< Perform an askReady.
    bool askReady();

    //!< Performa a digital Test.
    bool digitalTest(unsigned short *buff16, unsigned short value, unsigned short mode);

    //!< Load global configuration from file.
    int loadConfigGFromFile(QString filePath);

    //!< Load value in a global configuration register.
    bool loadConfigG(unsigned int reg, unsigned int value);

    //!< Read a global configuration register.
    bool readConfigG(unsigned int reg, unsigned int *buff);

    //!< Increase on one unit the ITHL register value.
    bool increaseITHL();

    //!< Decrease one one unit the ITHL register value.
    bool decreaseITHL();

    //!< Load a flat DACL value to all pixels in the detector.
    bool loadFlatConfigL(unsigned short value);

    //!< Load local configuration from file.
    int loadConfigLFromFile(QString filePath);

    //!< Read local configuration from detector.
    bool readConfigL(unsigned short *buff16);

    //!< Load expose paramter.
    bool loadExposureParameters(unsigned int numImages, unsigned int time, unsigned int waitingTimeBetweenImages, unsigned int overflowTime,
                              unsigned short inputSignals, unsigned short outputSignals, unsigned short acquisitionMode, unsigned short imageFormat,
                                unsigned int stackImages);

#ifdef PCI
    //!< Performs an asynchronous acquisition.
    int asyncAcquisition(IMG_TYPE type, unsigned int imageNumber, int burstNumber);

    //!< Performs a synchronous acquisition.
    int syncAcquisition(IMG_TYPE type, unsigned imageNumber, void** buff);
#elif USB
    //!< Performs an asynchronous acquisition.
    int asyncAcquisition(unsigned int burstNumber);

    //!< Performs a synchronous acquisition.
    int syncAcquisition(unsigned int burstNumber);
#endif

    //!< Generate a white image.
    int createWhiteImage(IMG_TYPE imageType, QString fileName);

    //!< Delete White Image
    bool deleteWhiteImage(QString whiteImageFilename);

    //!< Set the name of the white image used for flat field correction and dead pixel corrections.
    bool setWhiteImageFileName(QString fileName);

    //!< Return the current name of the white image used to do dead pixel corrections and flat field correction.
    QString getWhiteImageFileName();

    //!< Return the list of white images stored in the server.
    QStringList getWhiteImagesInDir();

    //!< Get the images store in the disk during asynchronous acquisition.
    bool getAsyncImagesFromDisk(IMG_TYPE imageType, unsigned int imageNumber, unsigned int burstNumber, unsigned int *imageBuffer);

    //!< Get the image counter value stored in the shared memory.
    int getImageCounter();

    //!< Read parameters.
    bool readGeometricalCorrectionParameters(QString fileName);

    //!< Create mask of dead and noisy pixels.
    void createDeadNoisyMask();

    //!< Reads temperature sensor if imXPAD.
    bool readTemperatureSensor(float* buff);

    //!< Image processing for XPAD detectors.
    bool processXpad(unsigned int *ret, bool flatFieldCorrectionFlag, bool geometricalCorrectionFlag, bool noisyPixelCorrectionFlag,
                     bool deadPixelCorrectionFlag, bool transferFlag, unsigned short burstNumber, unsigned short imageNumber,
                     unsigned short formatFile, QTcpSocket *tcpSocket, QString outputFilePath);

    //!< Performs a BEAM calibration.
    int calibrationBEAM(QString filePath, unsigned int exposureTime, unsigned int ITHLMax, unsigned int configuration);

    //!< Performs a Over-The-Noise Calibration using pulse.
    int calibrationOTNpulse(QString filePath, unsigned int configuration);

    //!< Performs a Over-The-Noise Calibration.
    int calibrationOTN(QString filePath, unsigned int configuration);

    //!< Get the detector status: Idle or Acquiring.
    int getDetectorStatus();

    //!< Reset the detector and clean the shared memory.
    bool resetDetector();

    //!< Get the status of abort flag
    bool getAbortStatus();

    //!< Abort current process.
    void abortCurrentProcess();

    //!< Clean shared memory.
    void cleanSharedMemory();

    //!< Clean the flags of abort in the driver.
    void cleanAbortProcess();

    //!< Clean images stored in the SSD Drive.
    void cleanSSDImages(unsigned int burstNumber, unsigned int imagesNumber);

    //!< Set value for High Voltage in the module.
    bool setHV(unsigned char DAC_HV);

    //!< Set parameters for cutting High Voltage in the module (used for CdTe detectors)
    bool setHVParameters(unsigned int hvCutTime, unsigned int hvDeadTime);

    void sendImage(QTcpSocket *tcpSocket);

    int inline getImageType(){
#ifdef PCI       
        return xpci_getImageFormat();
#elif USB
        return m_xpad->getImageType();
#endif
        }

    //!<
    void setLibStatus(int value){
#ifdef PCI
        xpci_setLibStatus(value);
#elif USB
        m_xpad->setLibStatus(value);
#endif
    }


private slots:

    //!< Read the mask for dead and noisy pixels.
    void readDeadNoisyMask();

    //!< Image processing for XPAD S10.
    bool processXpadS10(unsigned int *ret, bool flatFieldCorrectionFlag, bool geometricalCorrectionFlag, bool noisyPixelCorrectionFlag,
                        bool deadPixelCorrectionFlag, bool transferFlag, unsigned short burstNumber, unsigned short imageNumber,
                        unsigned short formatFile, QTcpSocket *tcpSocket, QString outputFilePath);

    //!< Image processing for XPAD S70.
    bool processXpadS70(unsigned int *ret, bool flatFieldCorrectionFlag, bool geometricalCorrectionFlag, bool noisyPixelCorrectionFlag,
                        bool deadPixelCorrectionFlag, bool transferFlag, unsigned short burstNumber, unsigned short imageNumber,
                        unsigned short formatFile, QTcpSocket *tcpSocket, QString outputFilePath);

    //!< Image processing for XPAD S140.
    bool processXpadS140(unsigned int *ret, bool flatFieldCorrectionFlag, bool geometricalCorrectionFlag, bool noisyPixelCorrectionFlag,
                         bool deadPixelCorrectionFlag, bool transferFlag, unsigned short burstNumber, unsigned short imageNumber,
                         unsigned short formatFile, QTcpSocket *tcpSocket, QString outputFilePath);

    //!< Image processing for XPAD S270.
    bool processXpadS210(unsigned int *ret, bool flatFieldCorrectionFlag, bool geometricalCorrectionFlag, bool noisyPixelCorrectionFlag,
                         bool deadPixelCorrectionFlag, bool transferFlag, unsigned short burstNumber, unsigned short imageNumber,
                         unsigned short formatFile, QTcpSocket *tcpSocket, QString outputFilePath);

    //!< Image processing for XPAD S270.
    bool processXpadS270(unsigned int *ret, bool flatFieldCorrectionFlag, bool geometricalCorrectionFlag, bool noisyPixelCorrectionFlag,
                         bool deadPixelCorrectionFlag, bool transferFlag, unsigned short burstNumber, unsigned short imageNumber,
                         unsigned short formatFile, QTcpSocket *tcpSocket, QString outputFilePath);

    //!< Image processing for XPAD S340.
    bool processXpadS340(unsigned int *ret, bool flatFieldCorrectionFlag, bool geometricalCorrectionFlag, bool noisyPixelCorrectionFlag,
                         bool deadPixelCorrectionFlag, bool transferFlag, unsigned short burstNumber, unsigned short imageNumber,
                         unsigned short formatFile, QTcpSocket *tcpSocket, QString outputFilePath);

    //!< Image processing for XPAD S540.
    bool processXpadS540(unsigned int *ret, bool flatFieldCorrectionFlag, bool geometricalCorrectionFlag, bool noisyPixelCorrectionFlag,
                         bool deadPixelCorrectionFlag, bool transferFlag, unsigned short burstNumber, unsigned short imageNumber,
                         unsigned short formatFile, QTcpSocket *tcpSocket, QString outputFilePath);

    //!< Image processing for XPAD S700.
    bool processXpadS700(unsigned int *ret, bool flatFieldCorrectionFlag, bool geometricalCorrectionFlag, bool noisyPixelCorrectionFlag,
                         bool deadPixelCorrectionFlag, bool transferFlag, unsigned short burstNumber, unsigned short imageNumber,
                         unsigned short formatFile, QTcpSocket *tcpSocket, QString outputFilePath);

    //!< Image processing for XPAD S1400.
    bool processXpadS1400(unsigned int *ret, bool flatFieldCorrectionFlag, bool geometricalCorrectionFlag, bool noisyPixelCorrectionFlag,
                          bool deadPixelCorrectionFlag, bool transferFlag, unsigned short burstNumber, unsigned short imageNumber,
                          unsigned short formatFile, QTcpSocket *tcpSocket, QString outputFilePath);




private:
    unsigned int    m_current_mask;                             //!< Variable to store the current current mask.

    unsigned short  m_detector_model;                           //!< Detector model.
    unsigned short  m_detector_sync_status;                     //!< Detector status for synchronous acquisition, 0 - Idle, 1 - Acquiring.
    unsigned short  m_chip_number;                              //!< Number of chips on the detector.
    unsigned short  m_chip_mask;                                //!< Mask for the number of chips.
    unsigned short  m_module_number;                            //!< Number of modules on the detector.
    unsigned short  m_module_number_from_file;                  //!< Number of modules by default.
    Image           *m_image;                                    //!< Instance of image class.

    double          m_angle[20];                                //!< Angle for each module in the detector.
    double          m_position_Y[20];                           //!< Position Y of the left upper corner of each module in the detector.
    double          m_position_X[20];                           //!< Position X of the left upper corner of each module in the detector.

    QString         m_output_file_path;                         //!< Output file path where images will be stored when TCP transfer was not selected.
    QString         m_white_image_file_name;                    //!< File name of the white image.

    float           *m_dead_noisy_image;                        //!< Buffer containing the mask of dead and noisy pixels.
    bool            m_dead_noisy_file_flag;                     //!< Flag indicating if mask for dead and noisy pixel file exists.
    bool            m_geometrical_corrections_file_flag;        //!< Flag indicating if geometrical correction file exists.
    bool            m_white_image_file_flag;                    //!< Flag indicating if white file was found.
    bool            m_show_timers_flag;                         //!> Flag used to turn on/off the timers for the image transfer and processing.

#ifdef USB
    XpadUSBLibrary  *m_xpad;
#endif
};

#endif // DETECTOR_H
