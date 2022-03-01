#include "image.h"

using namespace std;

Image::Image(){
    m_rows = 0;
    m_columns = 0;

    m_image = NULL;
}

Image::~Image(){
    delete[] m_image;
}

Image::Image(int numRows, int numCols, int value){
    m_rows = numRows;
    m_columns = numCols;

    m_image = new int [m_rows*m_columns];
    this->setAllPixelsToValue(value);
}

Image::Image(const Image& srcImage){
    m_rows = srcImage.m_rows;
    m_columns = srcImage.m_columns;

    if (m_image != NULL)
        delete[] m_image;
    m_image = new int [m_rows*m_columns];
    this->copyImage(0, 0, srcImage);
    //memcpy(this->m_image, srcImage.m_image, m_rows*m_columns*sizeof(qint32));
}

void Image::operator=(const Image& srcImage){
    m_rows = srcImage.m_rows;
    m_columns = srcImage.m_columns;

    if (m_image != NULL)
        delete[] m_image;
    m_image = new int [m_rows*m_columns];
    this->copyImage(0, 0, srcImage);
    //memcpy(this->m_image, srcImage.m_image, m_rows*m_columns*sizeof(qint32));
}

Image Image::operator*(const Image& secondImage){

    int row = this->m_rows;
    int col = this->m_columns;

    Image temp_image(row, col, -1);

    if (secondImage.m_rows<=row && secondImage.m_columns<=col)
        for (int i=0; i<row*col; i++)
            temp_image.setPixelValue(i, m_image[i] * secondImage.m_image[i]);

    return temp_image;
}

float Image::mean(){
    float total_value = 0;
    unsigned int counter = 0;

    for(int i = 0; i < m_rows*m_columns; i++)
        if (m_image[i] >= 0){
            total_value += m_image[i];
            counter++;
        }

    return (total_value / counter);
}

int Image::max(){
    int max = 0;

    for(int i = 0; i < m_rows*m_columns; i ++)
        if (m_image[i]>max)
            max = m_image[i];

    return max;
}


void Image::getImageInfo(int &numRows, int &numCols){
    numRows = m_rows;
    numCols = m_columns;
}

int Image::getPixelValue(int row, int col){
    return m_image[row * m_columns + col];
}

int Image::getPixelValue(int index){
    return m_image[index];
}

void Image::setPixelValue(int row, int col, int value){
    m_image[row * m_columns + col] = value;
}

void Image::setPixelValue(int index, int value){
    m_image[index] = value;
}

void Image::setAllPixelsToValue(int value){
    for(int i = 0; i < m_rows*m_columns; i++)
        m_image[i] = value;
}

void Image::setImageDimensions(int row, int col, int value){
    m_rows = row;
    m_columns = col;
    if (m_image != NULL)
        delete[] m_image;
    m_image = new int[m_rows*m_columns];
    this->setAllPixelsToValue(value);
}

void Image::copyImage(float y, float x, const Image& srcImage){

    int r1, c1;
    int row, col;

    row = srcImage.m_rows;
    col = srcImage.m_columns;

    r1 = qRound(y);
    c1 = qRound(x);

    if(r1 == 0 && c1 == 0 && m_rows == row && m_columns == col)
        memcpy(m_image, srcImage.m_image, m_rows*m_columns*sizeof(qint32));
    else{
        for (int j=0; j<row; j++){
            for (int i=0; i<col; i++){
                if (this->inBounds(j + r1, i + c1) && srcImage.m_image[j*col + i] != -5)
                    this->setPixelValue(j + r1, i + c1, srcImage.m_image[j*col + i]);
            }
        }
    }
}

bool Image::inBounds(int row, int col){
    if(row >= m_rows || row < 0 || col >=m_columns || col < 0)
        return false;
    else
        return true;
}

Image Image::getSubImage(int upperLeftRow, int upperLeftCol, int lowerRightRow,
                         int lowerRightCol){
    int width, height;

    width = lowerRightCol - upperLeftCol + 1;
    height = lowerRightRow - upperLeftRow + 1;

    Image temp_image(height, width, -1);

    if (upperLeftRow >=0 && lowerRightRow<m_rows && upperLeftCol>=0 && lowerRightCol<m_columns){
        unsigned int index = 0;
        for(int j = upperLeftRow; j <= lowerRightRow; j++)
            for(int i = upperLeftCol; i <= lowerRightCol; i++){
                //temp_image.m_image[(j - upperLeftRow)*width + (i - upperLeftCol)] = this->m_image[j*col + i];
                temp_image.setPixelValue(index, m_image[j*m_columns + i]);
                index++;
            }
    }

    return temp_image;

}

Image Image::enlargeImage(int value){
    int row, col, mean;
    int pixel;
    int enlarge_row, enlarge_col;

    row = this->m_rows * value;
    col = this->m_columns * value;

    Image temp_image(row, col, 0);

    for(int j = 0; j < this->m_rows; j++)
        for(int i = 0; i < this->m_columns; i++){
            pixel = this->m_image[j*this->m_columns + i];
            enlarge_row = j * value;
            enlarge_col = i * value;
            for(int c = enlarge_row; c < (enlarge_row + value); c++)
                for(int d = enlarge_col; d < (enlarge_col + value); d++)
                    temp_image.m_image[c*col + d] = pixel;
        }

    return temp_image;
}

Image Image::shrinkImage(int value){
    int rows, cols;

    rows = this->m_rows / value;
    cols = this->m_columns / value;

    Image temp_image(rows, cols, 0);

    for(int j = 0; j < rows; j++)
        for(int i = 0; i < cols; i++)
            temp_image.m_image[j*cols + i] = this->m_image[(j * value)*this->m_columns + (i * value)];

    return temp_image;
}

void Image::translateImage(float y, float x){
    int r1, c1;

    Image temp_image(m_rows, m_columns, 0);

    for(int j = 0; j < (m_rows - y); j++)
        for(int i = 0; i < (m_columns - x); i++){

            r1 = qRound(j + y);
            c1 = qRound(i + x);

            temp_image.m_image[r1*m_columns + c1] = this->m_image[j*m_columns + i];
        }

    this->copyImage(0, 0, temp_image);
    //for(int j = 0; j < m_rows; j++)
    //    for(int i = 0; i < m_columns; i++)
    //        this->m_image[j*m_columns + i] = temp_image.m_image[j*m_columns + i];

}

Image Image::rotateImage(float theta, int &Y0, int&X0){
    int r0, c0;
    int r1, c1;
    int row, col;
    int row_tmp, col_tmp;
    int row_min = INT_MAX, col_min = INT_MAX;
    int row_max = INT_MIN, col_max = INT_MIN;

    float rads = (theta * PI)/180.0;
    Y0 = 0;
    X0 = 0;

    row = this->m_rows;
    col = this->m_columns;

    //Rotation center
    r0 = 0;
    c0 = 0;

    for(int r = 0; r < row; r=r+row-1)
        for(int c = 0; c < col; c=c+col-1){
            row_tmp = qRound(r0 + ((r - r0) * qCos(rads)) - ((c - c0) * qSin(rads)));
            col_tmp = qRound(c0 + ((r - r0) * qSin(rads)) + ((c - c0) * qCos(rads)));

            if (row_tmp < row_min)
             row_min = row_tmp;
            if (col_tmp < col_min)
             col_min = col_tmp;
            if (row_tmp > row_max)
             row_max = row_tmp;
            if (col_tmp > col_max)
             col_max = col_tmp;
        }

    int final_col_number = col_max - col_min + 1;
    int final_row_number = row_max - row_min + 1;

    //cout << "Final image size = " << final_col_number << " " << final_row_number << endl;

    Image temp_image(final_row_number, final_col_number, -5);

    int r,c;
    for(int r1 = 0; r1 < final_row_number; r1++)
        for(int c1 = 0; c1 < final_col_number; c1++){

            r = qRound(((r1+row_min) * qCos(rads)) + ((c1+col_min) * qSin(rads)));
            c = qRound((-(r1+row_min) * qSin(rads)) + ((c1+col_min) * qCos(rads)));

            if (this->inBounds(r,c))
                temp_image.m_image[r1*(final_col_number) + c1] = this->m_image[r*col + c];

            if ( r == 0 && c == 0){
                Y0 = r1;
                X0 = c1;
            }
        }


    return temp_image;
}

void Image::flipImage(bool flagVertical){
    int row =this->m_rows;
    int col = this->m_columns;

    Image temp_image(row,col,0);

    if(flagVertical == true){

        for(int j = 0; j < row; j++)
            for(int i = 0; i < col; i++)
                temp_image.m_image[(row - (j + 1))*col + i] = this->m_image[j*col + i];
    }else{
        for(int j = 0; j < row; j++)
            for(int i = 0; i < col; i++)
                temp_image.m_image[j*col + (col - i - 1)] = this->m_image[j*col + i];
    }

    this->copyImage(0, 0, temp_image);
}

bool Image::readImageFromDatFile(QString fileName){

    if (fileName.isEmpty())
        return false;

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        cout << "ERROR: Image::" <<  __func__ << " ---> inputFilePath doesn't exists." << endl;
        cout << "\t" << fileName.toStdString().c_str() << endl;
        return false;
    }else{
        QByteArray buffer = file.readAll();
        QString data = buffer;
        QStringList values = data.split(QRegExp("[ \t\n]"), QString::SkipEmptyParts);
        for (int i=0; i<values.size(); i++){
            this->setPixelValue(i,values[i].toInt());
            //cout << values[i].toInt() << " ";
        }
        file.close();
        return true;
    }
}

void Image::searchNoisyPixels(){

    int row = this->m_rows;
    int col = this->m_columns;

    for (int i=0; i<row*col; i++)
        if (this->getPixelValue(i) > 10)
            this->setPixelValue(i,-3);
        else
            this->setPixelValue(i,1);

}

void Image::searchDeadPixels(float mean){
    int row = this->m_rows;
    int col = this->m_columns;

    unsigned int seuil = qRound(mean/5);

    for(int j=0; j<row; j++)
        for(int i=0; i<col; i++)
            if (((i+1) % IMG_COLUMN) != 0 && (i % IMG_COLUMN) != 0)
                if (this->getPixelValue(j, i) < seuil)
                    this->setPixelValue(j, i, -2);
                else
                    this->setPixelValue(j, i,1);
            else
                if (this->getPixelValue(j, i) < 2.5*seuil)
                    this->setPixelValue(j, i, -2);
                else
                    this->setPixelValue(j, i, 1);
}

bool Image::saveImageToDatFile(QString fileName){

    int row = this->m_rows;
    int col = this->m_columns;

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)){
        QTextStream out(&file);

        for (int j=0; j<row; j++){
            for (int i=0; i<col; i++)
                out << this->getPixelValue(j,i) << " ";
            out << "\n";
        }
        file.close();
        return true;

    }else{
        cout << "ERROR: Image::" <<  __func__ << " ---> fileName could not be opened." << endl;
        cout << "\t" << fileName.toStdString().c_str() << endl;
        return false;
    }
}

bool Image::readGeometricalParameters(QString fileName){

    QString     data;
    QStringList list;


    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        cout << "ERROR: Image::" <<  __func__ << " ---> fileName doesn't exists." << endl;
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
                m_angle[i] = list.at(3).toDouble();
                i++;
            }
        }
        file.close();
        //cout << "Position X = " << position_X[i] <<  " Position Y = " << position_Y[i] << " Position Y = " << angle[i] <<endl;
        return true;
    }
}

void Image::deadNoisyCorrection(unsigned int *ret, float *deadNoisy, bool deadNoisyFileFlag, bool noisyPixelCorrectionFlag, bool deadPixelCorrectionFlag, bool flatFieldCorrectionFlag){

    int row, col;

    //cout << __func__ << endl;

    row = this->m_rows;
    col = this->m_columns;

    float mean = 0;

    if (deadNoisyFileFlag && (deadPixelCorrectionFlag  || noisyPixelCorrectionFlag)){
        //cout << "Dead_Noisy Flag = ON" << endl;
        for(int j=0; j<row; j++)
            for(int i=0; i<col; i++)
                //if (j>=1 && j<row-1 && i>=1 && i<col-1){
                    if ( (deadNoisy[j*col+i] == -2 && deadPixelCorrectionFlag) || (deadNoisy[j*col+i] == -3 && noisyPixelCorrectionFlag) ){
                        mean = 0;
                        int count = 0;
                        for(int y=j-1;y<=j+1; y++)
                            for(int x=i-1; x<=i+1; x++)
                                if (x!=i && y!=j && x>=0 && y >=0 && x<col && y<row && deadNoisy[y*col+x]>=0){
                                    if (((x+1) % IMG_COLUMN) != 0 && (x % IMG_COLUMN) != 0){
                                        if (flatFieldCorrectionFlag)
                                            mean += ret[y*col+x] * deadNoisy[y*col+x];
                                        else
                                            mean += ret[y*col+x];
                                    }
                                    else{
                                        if (flatFieldCorrectionFlag)
                                            mean += ret[y*col+x] * deadNoisy[y*col+x];
                                        else
                                            mean += ret[y*col+x] * 0.4065 ;
                                    }
                                    count ++;
                                }
                        if(count>0)
                            mean = mean / count;
                        //cout << "Pixel " << i << "," << j << " corrected. Mean = " << mean << endl;
                        this->setPixelValue(j,i,qRound(mean));
                        //cout << "Pixel " << i << "," << j << " corrected. Mean = " << mean << endl;
                    }
                    else{
                        if (flatFieldCorrectionFlag)
                            this->setPixelValue(j,i,qRound(ret[j*col+i] * deadNoisy[j*col+i]));
                        else
                            this->setPixelValue(j,i,ret[j*col+i]);
                    }
                /*}
                else{
                    if (flatFieldCorrectionFlag && deadNoisy[j*col+i]>=0)
                        this->setPixelValue(j,i,qRound(ret[j*col+i] * deadNoisy[j*col+i]));
                    else
                        this->setPixelValue(j,i,ret[j*col+i]);
                }*/
    }else{
        //cout << "Dead_Noisy Flag = OFF" << endl;
        for(int i=0; i< row*col; i++)
            if (flatFieldCorrectionFlag && deadNoisy[i]>=0)
                this->setPixelValue(i,qRound(ret[i] * deadNoisy[i]));
            else if (deadNoisy[i] < 0)
                this->setPixelValue(i,deadNoisy[i]);
            else
                this->setPixelValue(i,ret[i]);
    }
}

Image Image::expandDoublePixelsNoBorders(bool flatFieldCorrectionFlag){

    int row, col;
    int row1, col1;
    int valueBefore, value;

    row = this->m_rows;
    col = this->m_columns;

    int chipNumber = col/IMG_COLUMN;

    row1 = row;
    col1 = col + (chipNumber-1)*3;

    Image temp_image(row1-2,col1-2,0);

    int count = 0;
    for(int j=1; j<row-1; j++)
        for(int i=1; i<col-1; i++){

            if ( ((i+1) % IMG_COLUMN) != 0 && (i % IMG_COLUMN) != 0 )
                temp_image.m_image[count] =m_image[j*col + i];

            else if( ((i+1) % IMG_COLUMN) == 0  && i!=col-1){
                //cout << "row = "  << j << " col = " << i + 1 << endl;
                if (!flatFieldCorrectionFlag && m_image[j*col + i] >= 0)
                    valueBefore = qRound(m_image[j*col + i]*0.4065);
                else
                    valueBefore = m_image[j*col + i];

                //cout << valueBefore << " " << m_image[j*col + i] << endl;

                temp_image.m_image[count] = valueBefore;
                count++;
                temp_image.m_image[count] = valueBefore;
            }
            else if((i % IMG_COLUMN) == 0 && i!=0){
                //cout << "row1 = "  << j << " col1 = " << i << endl;
                if(!flatFieldCorrectionFlag && m_image[j*col + i]>=0)
                    value = qRound(m_image[j*col + i]*0.4065);
                else
                    value = m_image[j*col + i];

                //cout << value << " " << m_image[j*col + i] << endl;

                if (value >=0 && valueBefore >=0)
                    temp_image.m_image[count] = qRound((value + valueBefore)*0.5);
                else if (value < 0 && valueBefore >= 0)
                    temp_image.m_image[count] = valueBefore;
                else if (value >= 0 && valueBefore < 0)
                    temp_image.m_image[count] = value;
                else
                    temp_image.m_image[count] = -1;

                count++;
                temp_image.m_image[count] = value;
                count++;
                temp_image.m_image[count] = value;
            }
            count++;
        }

    //cout << count << endl;

    return temp_image;
}

Image Image::expandDoublePixels(bool flatFieldCorrectionFlag){

    int row, col;
    int row1, col1;
    int valueBefore, value;

    row = this->m_rows;
    col = this->m_columns;

    int chipNumber = col/IMG_COLUMN;

    row1 = row;
    col1 = col + (chipNumber-1)*3;

    Image temp_image(row1,col1,0);

    //cout << "row1 = "  << temp_image.rows << " col1 = " << temp_image.columns << endl;

    int count = 0;
    for(int j=1; j<row-1; j++)
        for(int i=1; i<col-1; i++){

            if ( ((i+1) % IMG_COLUMN) != 0 && (i % IMG_COLUMN) != 0 )
                temp_image.m_image[count] =m_image[j*col + i];

            else if( ((i+1) % IMG_COLUMN) == 0  && i!=col-1){
                //cout << "row = "  << j << " col = " << i + 1 << endl;
                if (!flatFieldCorrectionFlag && m_image[j*col + i] >= 0)
                    valueBefore = qRound(m_image[j*col + i]*0.4065);
                else
                    valueBefore = m_image[j*col + i];

                //cout << valueBefore << " " << m_image[j*col + i] << endl;

                temp_image.m_image[count] = valueBefore;
                count++;
                temp_image.m_image[count] = valueBefore;
            }
            else if((i % IMG_COLUMN) == 0 && i!=0){
                //cout << "row1 = "  << j << " col1 = " << i << endl;
                if(!flatFieldCorrectionFlag && m_image[j*col + i]>=0)
                    value = qRound(m_image[j*col + i]*0.4065);
                else
                    value = m_image[j*col + i];

                //cout << value << " " << m_image[j*col + i] << endl;

                if (value >=0 && valueBefore >=0)
                    temp_image.m_image[count] = qRound((value + valueBefore)*0.5);
                else if (value < 0 && valueBefore >= 0)
                    temp_image.m_image[count] = valueBefore;
                else if (value >= 0 && valueBefore < 0)
                    temp_image.m_image[count] = value;
                else
                    temp_image.m_image[count] = -1;

                count++;
                temp_image.m_image[count] = value;
                count++;
                temp_image.m_image[count] = value;
            }
            count++;
        }

    //cout << count << endl;

    return temp_image;
}

bool Image::saveImage(ushort burstNumber, ushort imageNumber, ushort formatFile, QString outputFilePath){

    if (QDir(outputFilePath).exists()){

        QString file_name;
        if (formatFile == 1)
            file_name = outputFilePath + "burst_" + QString::number(burstNumber) + "_image_" + QString::number(imageNumber) + ".bin";
        else
            file_name = outputFilePath + "burst_" + QString::number(burstNumber) + "_image_" + QString::number(imageNumber) + ".dat";

        //cout << "\t" << file_name.toStdString().c_str() << endl;

        if (formatFile == 1){

            QFile file(file_name);

            quint32 data_size = m_rows*m_columns*sizeof(qint32);

            if(file.open(QIODevice::WriteOnly)){
                file.write((char *)m_image, data_size);
                file.close();
            }
            else{
                cout << "ERROR: Image::" <<  __func__ << " ---> fileName could not be opened." << endl;
                cout << "\t" << file_name.toStdString().c_str() << endl;
                return false;
            }
        }
        else{
            if (!this->saveImageToDatFile(file_name))
                return false;
        }
        return true;
    }else{
        cout << "ERROR: Image::" <<  __func__ << " ---> outputFilePath doesn't exists." << endl;
        cout << "\t" << outputFilePath.toStdString().c_str() << endl;
        return false;
    }
}

bool Image::transferImage(QTcpSocket *tcpSocket){

    // Vars
    timeval start, end;
    double tg1, tg2, tg3;

    quint32 data_size = m_rows*m_columns*sizeof(qint32);



    if (tcpSocket->state() == QTcpSocket::ConnectedState){

        // Timing
        gettimeofday(&start, NULL);
        tg1 = start.tv_sec + start.tv_usec / 1000000.0;

        tcpSocket->write((char *)&data_size, sizeof(qint32));
        tcpSocket->write((char *)&m_rows, sizeof(qint32));
        tcpSocket->write((char *)&m_columns, sizeof(qint32));
        tcpSocket->write((char *)m_image, data_size);
        tcpSocket->waitForBytesWritten();

        gettimeofday(&end, NULL);
        tg2 = end.tv_sec + end.tv_usec / 1000000.0;


        tcpSocket->waitForReadyRead();
        tcpSocket->readLine();

        gettimeofday(&end, NULL);
        tg3 = end.tv_sec + end.tv_usec / 1000000.0;

        cout << "\t--->Datasize [Bytes]: " << data_size << endl;
        cout << "\t--->Time sending one S1400 image [s]: " << tg2 -tg1 << endl;
        cout << "\t--->Transfer speed [Mbits per second]: " << (data_size*8/(tg2 -tg1))/1000000 << endl;
        cout << "\t--->Time receiving ACK for one S1400 image [s]: " << tg3 -tg1 << endl;
        cout << "\t--->Receiver speed [Mbits per second]: " << (data_size*8/(tg3 -tg1))/1000000 << endl;



        return true;
    } else{
        cout << "ERROR: Image::" <<  __func__ << " ---> tcp socket broken." << endl;
        return false;
    }
}


