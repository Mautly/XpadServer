#ifndef MAINTHREAD_H
#define MAINTHREAD_H

#include <QThread>
#include <QtNetwork/QTcpSocket>
#include <QCoreApplication>
#include <QStringList>
#include <QDir>
#include <QtCore/qmath.h>
#include <QDataStream>
#include <sys/time.h>

#include "servertype.h"

#ifdef PCI
#include <xpci_interface.h>
#include <xpci_interface_expert.h>
#include <xpci_time.h>
#include <xpci_imxpad.h>
#include <xpci_calib_imxpad.h>
#elif USB
#include <xpadusblibrary.h>
#endif


#include "imageprocessingthread.h"
#include "detector.h"

#ifdef USB
#include <xpadusblibrary.h>
#endif

#define IMG_LINE    120
#define IMG_COLUMN  80

class MainThread : public QThread
{
    Q_OBJECT

#ifdef PCI
public:
    MainThread(qintptr socketDescriptor, const QString &message, QObject *parent);
#elif USB
public:
    MainThread(qintptr socketDescriptor, const QString &message, XpadUSBLibrary *XPAD, QObject *parent);
#endif
    ~MainThread();

private:
    void run();

signals:
    void error(QTcpSocket::SocketError socketError);

private slots:


    void receivingCommands();                                   //!< Server main loop that interprets commands from TCP client.
    void sendOK();                                              //!< Sends "0" through TCP socket to indicate operation finished succesfully.
    void sendError(QString message);                            //!< Sends "-1" through TCP socket as well as the an error message.
    void sendWarning(QString message);                          //!< Sends "1" through TCP socket as well as the an error message.
    void sendMessage(QString message);                          //!< Sends message through TCP socket.
    void sendIntValue(int value);                               //!< Sends an integer value through TCP socket.
    void transferEmptyImage();                                  //!< Sends an empty image through TCP socket.
    bool transferParametersFile(u_int16_t *ret);                //!< Sends a file through TCP socket.
    bool receiveParametersFile(QString fileName);               //!< Receives file through TCP socket and save it in a file.
    unsigned int getRegNumberFromRegName(QString registerName); //!< Get the register number corresponding to the register name.
    void getUSBDeviceList();                                    //!< Functions available in the server.
    void setUSBDevice();                                        //!< Set the USB device to be used by the server.

    void init();
    void getFirmwareID();
    //void setDetectorModel();
    void getDetectorModel();
    void getDetectorType();
    void getModuleNumber();
    void getChipNumber();
    void getModuleMask();
    void getChipMask();
    void setModuleMask();
    void getImageSize();
    void askReady();
    void digitalTest();
    void loadDefaultConfigG();
    void loadConfigGFromFile();
    void loadConfigG();
    void readConfigG();
    void ITHLIncrease();
    void ITHLDecrease();
    void loadFlatConfigL();
    void loadConfigLFromFile();
    void readConfigL();
    void setExposureParameters();
    void startExposure();
    void createWhiteImage();
    void deleteWhiteImage();
    void setWhiteImage();
    void getWhiteImagesInDir();
    void calibrationBEAM();
    void calibrationOTNPulse();
    void calibrationOTN();
    void getDetectorStatus();
    void resetDetector();
    void getBurstNumber();
    void abortCurrentProcess();
    void exit();

    void setImageNumber();
    void getImageNumber();
    void setExposureTime();
    void getExposureTime();
    void setOverflowTime();
    void getOverflowTime();
    void setWaitingTimeBetweenImages();
    void getWaitingTimeBetweenImages();
    void setGeometricalCorrectionFlag();
    void getGeometricalCorrectionFlag();
    void setFlatFieldCorrectionFlag();
    void getFlatFieldCorrectionFlag();
    void setNoisyPixelCorrectionFlag();                 //!< Set flag for noisy pixels correction.
    void getNoisyPixelCorrectionFlag();                 //!< Retreives the flag value for noisy pixels correction.
    void setDeadPixelCorrectionFlag();                  //!< Set flag for dead pixels correction.
    void getDeadPixelCorrectionFlag();                  //!< Retreives the flag value for dead pixels correction.
    void setImageTransferFlag();
    void getImageTransferFlag();
    void setAcquisitionMode();
    void getAcquisitionMode();
    void setOutputFormatFile();
    void getOutputFormatFile();
    void setOutputFilePath();
    void getOutputFilePath();
    void setInputSignal();
    void getInputSignal();
    void setOutputSignal();
    void getOutputSignal();
    void readDetectorTemperature();
    void setHV();
    void setHVParameters();

    void setDebugMode();
    void showTimers();

    void processImages(IMG_TYPE imageType);
    void sendImage();

private:

    int             m_socket_descriptor;                //!< Socket descriptor used to use the TCP socket in the thread.
    QString         m_text;                             //!< Use to transfer the a message to the TCP socket in the thread.
    QTcpSocket      *m_tcp_socket;                      //!< Instance of TCP Socket.

    QByteArray      m_buffer;                           //!< Buffer used to store the command received from the TCP client.
    bool            m_quit_flag;                        //!< Flag used to quit the receivingCommands infinit loop.

    unsigned int    m_number_images;                    //!< Variable used to store the number of images required in Expose.
    unsigned int    m_exposure_time;                    //!< Variable used to store the exposure time, time is given in [us].
    unsigned int    m_waiting_time_between_images;      //!< Variable used to store the time between images, normally 5000 [us].
    unsigned int    m_overflow_time;                    //!< Variable used to store the overflow time.
    unsigned int    m_input_signal;                     //!< Trigger signal used during exposure.
    unsigned int    m_output_signal;                    //!< Output signal used dureing exposure.

    //unsigned short  m_xpad_detector_model;              //!< Variable used to store the detector model currently used.

    Detector        *m_detector;                         //!< Instance of detector class.

    int             m_burst_number;                     //!< Variable used to track the current burst number.
    unsigned int    m_current_mask;                     //!< Variable to store the current current mask.

    bool            m_geometrical_correction_flag;      //!< Flag used to indicate if geometrical corrections will be applied.
    bool            m_flat_field_correction_flag;       //!< Flag used to indicate if flat field corrections will be applied.
    bool            m_noisy_pixel_correction_flag;      //!< Flag used to indicate if noisy pixel corrections will be applied.
    bool            m_dead_pixel_correction_flag;       //!< Flag used to indicate if dead pixel corrections will be applied.
    bool            m_image_transfer_flag;              //!< Flag used to indicate if image will be transfer through TCP socket.
    unsigned int    m_image_format;                     //!< Variable used to set image format: 16 bits or 32 bits.
    unsigned short  m_acquisition_mode;                 //!< Variable used to set the acquisition mode: Normal, Burst-DDR, Burst-SSD.
    unsigned short  m_output_format_file;               //!< Variable used to set the output format of the file, 0.- ASCII, 1.-binary.
    QString         m_output_file_path;                 //!< Variable used to store the output path where the image will be stored in the SERVER.
    QString         m_white_image_file_name;            //!< Variable to store the name of the white image.
    unsigned int    m_stack_images;                     //!< Variable used to store the images / stack that will be acquired.

    ImageProcessingThread *m_processing_thread;         //!< Processing thread launched in paralled to process images and send them to Client while freeing the Server.

#ifdef USB
    XpadUSBLibrary  *m_xpad;
#endif
};

#endif // MAINTHREAD_H
