#include "imageprocessingthread.h"

using namespace std;

ImageProcessingThread::ImageProcessingThread(Detector *detector, IMG_TYPE imageType, int burstNumber, int numImages,
                                             bool geometricalCorrectionsFlag, bool flatFieldCorrectionsFlag,
                                             bool noisyPixelCorrectionsFlag, bool deadPixelCorrectionsFlag,
                                             bool imageTransferFlag, unsigned short formatFile, QString outputFilePath):
    m_detector(detector), m_image_type(imageType), m_burst_number(burstNumber), m_number_images(numImages),
    m_geometrical_corrections_flag(geometricalCorrectionsFlag), m_flat_field_corrections_flag(flatFieldCorrectionsFlag),
    m_noisy_pixel_correction_flag(noisyPixelCorrectionsFlag), m_dead_pixel_correction_flag(deadPixelCorrectionsFlag),
    m_image_transfer_flag(imageTransferFlag), m_format_file(formatFile), m_output_file_path(outputFilePath){
}

void ImageProcessingThread::run(){

    // Vars
    timeval start, end;
    double tg1, tg2, tg3, mean_get_time = 0, mean_process_time = 0;

    //Force to save files in Binary for image transfer via SSD.
    m_format_file = 1;

    int last_acquired_imageNumber = 0;
    int module_number = m_detector->getModuleNumber();
    unsigned int *fast_ret = new unsigned int[120*560*module_number];

    int image_counter = 0;
    while (image_counter < m_number_images){

        last_acquired_imageNumber = m_detector->getImageCounter();
        //cout << "\tImageProcessingThread ---> " <<__func__ << " Last acquired image: " << last_acquired_imageNumber << "\r";

        // Timing
        gettimeofday(&start, NULL);
        tg1 = start.tv_sec + start.tv_usec / 1000000.0;

        if (image_counter < last_acquired_imageNumber){
            if (m_detector->getAsyncImagesFromDisk(m_image_type, image_counter, m_burst_number, fast_ret)){

                gettimeofday(&end, NULL);
                tg2 = end.tv_sec + end.tv_usec / 1000000.0;
                mean_get_time += tg2-tg1;

                if (!m_detector->processXpad(fast_ret, m_flat_field_corrections_flag, m_geometrical_corrections_flag, m_noisy_pixel_correction_flag, m_dead_pixel_correction_flag, m_image_transfer_flag, m_burst_number, image_counter, m_format_file, m_tcp_socket, m_output_file_path)){
                    image_counter = m_number_images;
                }

                gettimeofday(&end, NULL);
                tg3 = end.tv_sec + end.tv_usec / 1000000.0;
                mean_process_time += tg3-tg2;

                if (!m_detector->getAbortStatus())
                    image_counter++;
                else{
                    m_detector->cleanSSDImages(m_burst_number, m_number_images);
                    m_detector->cleanAbortProcess();
                    delete[] fast_ret;
                    return;
                }
            }else{
                if (m_detector->getAbortStatus()){
                    m_detector->cleanSSDImages(m_burst_number, m_number_images);
                    m_detector->cleanAbortProcess();
                    delete[] fast_ret;
                    return;
                }
            }
        }
    }

    m_detector->cleanSSDImages(m_burst_number, m_number_images);
    m_detector->cleanAbortProcess();

    // Reporting Statistics
    cout << "Mean time to get one image from disk [s]: " << mean_get_time/m_number_images << endl;
    cout << "Mean time to process and save one image [s]: " << mean_process_time/m_number_images << endl;

    delete[] fast_ret;
}
