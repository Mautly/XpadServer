#ifndef IMAGEPROCESSINGTHREAD_H
#define IMAGEPROCESSINGTHREAD_H

#include <QThread>
#include <QDir>

#include "detector.h"

class ImageProcessingThread : public QThread
{
    Q_OBJECT

public:
    //!< Thread for Acquire and images and transmit it to the client.
    ImageProcessingThread(Detector *detector, IMG_TYPE imageType, int burstNumber, int numImages,
                          bool geometricalCorrectionsFlag, bool flatFieldCorrectionsFlag,
                          bool noisyPixelCorrectionsFlag, bool deadPixelCorrectionsFlag,
                          bool imageTransferFlag, unsigned short formatFile, QString outputFilePath);

private:
    void run();

private:
    Detector        *m_detector;                                //!< Instance of detector class.

    int             m_current_mask;                             //!< Variable to store the current current mask.
    IMG_TYPE        m_image_type;                               //!< Image type: 16-bits or 32-bits
    int             m_burst_number;                             //!< Current burst number.
    int             m_number_images;                            //!< Number of images to acquire.
    bool            m_geometrical_corrections_flag;             //!< Geometrical correction flag received from expose parameters.
    bool            m_flat_field_corrections_flag;              //!< Flat field correction flag received from expose parameters.
    bool            m_noisy_pixel_correction_flag;              //!< Flag used to indicate if noisy pixel corrections will be applied.
    bool            m_dead_pixel_correction_flag;               //!< Flag used to indicate if dead pixel corrections will be applied.
    bool            m_image_transfer_flag;                      //!< Image transfer flag received from expose parameters.
    unsigned short  m_format_file;                              //!< Format file received from expose parameters.
    int             m_socket_descriptor;
    QTcpSocket      *m_tcp_socket;                              //!< Current TCP socket used in the connections with the current client.
    QString         m_output_file_path;                         //!< File path where images will be stored if transfer is not selected.
};

#endif // IMAGEPROCESSINGTHREAD_H
