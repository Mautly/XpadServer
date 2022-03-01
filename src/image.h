#ifndef IMAGE_H
#define IMAGE_H

#include <QtGlobal>
#include <QtMath>
#include <QDir>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QTextStream>
#include <QTcpSocket>
#include <iostream>

#include <sys/time.h>

#define PI          3.14159265359
#define IMG_LINE    120
#define IMG_COLUMN  80

class Image
{
public:
    Image();    
    ~Image();
    Image(int numRows, int numCols, int value);
    Image(const Image& srcImage);
    void operator=(const Image& srcImage);

    //!< Multiplication of two images.
    Image operator*(const Image& secondImage);

    //!< Estimates the mean value of all pixels.
    float mean();

    //!< Estimates the max value of all pixels.
    int max();

    //!< Returns the size and initial pixel value of the image.
    void getImageInfo(int &numRows, int &numCols);

    //!< Returns the value of a single pixel identified by the row and column number.
    int getPixelValue(int row, int col);

    //!< Returns the value of a single pixel identified by the matrix index.
    int getPixelValue(int index);

    //!< Sets the value of a single pixel specifing the row and column number.
    void setPixelValue(int row, int col, int value);

    //!< Sets the value of a single pixel specifing the image matrix index.
    void setPixelValue(int index, int value);

    //!< Sets all pixels to a single value.
    void setAllPixelsToValue(int value);

    //!< Sets the dimensions of the image.
    void setImageDimensions(int row, int col, int value);

    //!< Copies on region of an images into the current image.
    void copyImage(float y, float x, const Image& srcImage);

    //!< Verifies if the row and column specified is not greater than image size.
    bool inBounds(int row, int col);

    //!< Gets a region of an image specified the coordinnates of the superior left pixel position and the inferior right pixel position.
    Image getSubImage(int upperLeftRow, int upperLeftCol,
                      int lowerRightRow, int lowerRightCol);

    //!< Performs a zoom in of the image.
    Image enlargeImage(int value);

    //!< Performs a zoom out of the image.
    Image shrinkImage(int value);

    //!< Translate the image up, down, left or right.
    void translateImage(float y, float x);

    //!< Rotates the images keeping the same image dimensions.
    Image rotateImage(float theta, int &Y0, int&X0);

    //!< Performs an horizontal reflexion if flag is true or vertical reflexion if flag is false.
    void flipImage(bool flagVertical);

    //!< Read image from file.
    bool readImageFromDatFile(QString fileName);

    //!< Searchs for noisy pixels and mark them with -3 value.
    void searchNoisyPixels();

    //!< Searchs for dead pixels and mark them with -2 value.
    void searchDeadPixels(float mean);

    //!< Save current image into a file.
    bool saveImageToDatFile(QString fileName);

    //!< Reads geometrical correction parameters from file.
    bool readGeometricalParameters(QString fileName);

    //!< Performs dead and noisy corrections on the image.
    void deadNoisyCorrection(unsigned int *ret, float *deadNoisy, bool deadNoisyFileFlag, bool noisyPixelCorrectionFlag, bool deadPixelCorrectionFlag, bool flatFieldCorrectionFlag);

    //!< Expands the double pixels located in chips borders and removes pixels on image border.
    Image expandDoublePixelsNoBorders(bool flatFieldCorrectionFlag);

    //!< Expands double pixels located in chips borders.
    Image expandDoublePixels(bool flatFieldCorrectionFlag);

    //!< Saves current image into file specifying image dimensions and burst and image number.
    bool saveImage(ushort burstNumber, ushort imageNumber, ushort formatFile, QString outputFilePath);

    //!< Transfer current image through a TCP socket.
    bool transferImage(QTcpSocket *tcpSocket);


private:
    quint32          m_rows;             //!< Rows number of the image
    quint32          m_columns;          //!< Columns number of the image
    qint32          *m_image;           //!< Pointer to current image

    float           m_angle[20];        //!< Angles of each module in current detector.
    float           m_position_Y[20];   //!< Y positions of each module in current detector.
    float           m_position_X[20];   //!< x positions of each module in current detector.

};

#endif // IMAGE_H
