#include "Filters.h"

///////////////////////////////////////////////////////////////////////
//
//
// Раздел фильтров
//
//
///////////////////////////////////////////////////////////////////////


void Canny(int** LightMap, long width_, long height_)
{
    int i, j;
    int** LightMap2;
    LightMap2 = new int*[height_];
    for (i = 0; i < height_; i++) 
    {
        LightMap2[i] = new int[width_];
    }

//*****************************************************************
//                  фильтр гаусса(размытие)
//*****************************************************************

    GaussianBlurring(LightMap, width_, height_);

//*****************************************************************
//                  поиск градиентов(оператором Собеля)
//*****************************************************************

    double sobelX[3][3];
    double sobelY[3][3];
    double G = 0;
    double Gx = 0;
    double Gy = 0;
    double temp = 0;
    double o0 = 0;
    double** dop = new double* [height_];
    for(i = 0; i < height_; i++)
    {
        dop[i] = new double [width_];
    }


    // Маска для оператора Прюитт. Меняем средние 1 на 2 и получаем маску для Собеля
    sobelX[0][0] = -1;  sobelX[1][0] = 0;  sobelX[2][0] = 1;
    sobelX[0][1] = -1;  sobelX[1][1] = 0;  sobelX[2][1] = 1;
    sobelX[0][2] = -1;  sobelX[1][2] = 0;  sobelX[2][2] = 1;

    sobelY[0][0] = -1;  sobelY[1][0] = -1; sobelY[2][0] = -1;
    sobelY[0][1] = 0;   sobelY[1][1] = 0;  sobelY[2][1] = 0;
    sobelY[0][2] = 1;   sobelY[1][2] = 1;  sobelY[2][2] = 1;


    for (i = 0; i < height_-1; i++)
    {
        for (j = 0; j < width_-1; j++)
        {
            Gx = 0;
            Gy = 0;
            temp = 0;
            G = 0;
            for (int m = i-1; m <= i+1; m++)
            {
                for (int n = j-1; n <= j+1; n++)
                {
                    if (m < 0 || m >= height_ || n < 0 || n >= width_)
                    {
                            temp = 0;
                    }
                    else
                    {
                            temp = LightMap[m][n];
                    }
                    Gx += (temp * sobelX[n+1-j][m+1-i]);
                    Gy += (temp * sobelY[n+1-j][m+1-i]);
                }
            }
            G = sqrt(Gx*Gx + Gy*Gy); // alternative - abs(Gx + Gy)
            
            o0 = atan2(Gy, Gx);

            LightMap2[i][j] = round(G);
            dop[i][j] = o0;
        }
    }

//*****************************************************************
//                  подавление не максимумов
//*****************************************************************

    for (i = 1; i < height_ - 1; i++)
    {
        for (j = 1; j < width_ - 1; j++)
        {
            LightMap[i][j] = LightMap2[i][j];
        }
    }

    int ax, ay;
    int bx, by;

    for (i = 1; i < height_ - 1; i++) 
    {
        for (j = 1; j < width_ - 1; j++)
        {
            if (dop[i][j] < 0)
                dop[i][j] += M_PI;

            if (dop[i][j] <= M_PI / 8)
            {
                ax = 0, ay = 1;
                bx = 0, by = -1;
            }   else if (dop[i][j] <= 3*M_PI/8) 
            {
                ax = 1, ay = 1;
                bx = -1, by = -1;
            }   else if (dop[i][j] <= 5*M_PI/8)
            {
                ax = 1, ay = 0;
                bx = -1, by = 0;
            }   else if (dop[i][j] <= 7*M_PI/8) 
            {
                ax = -1, ay = 1;
                bx = 1, by = -1;
            }   else
            {
                ax = 0, ay = 1;
                bx = 0, by = -1;
            }

            if (LightMap2[i+ax][j+ay] > LightMap2[i][j])
            {
                LightMap[i][j] = 0;
                continue;
            }
            if (LightMap2[i+bx][j+by] > LightMap2[i][j])
            {
                LightMap[i][j] = 0;
                continue;
            }
        }
    }

//**********************************************************************************************************
//                  первый шаг двойной фильтрации(после него и начинается выделение областей  а не границ)
//**********************************************************************************************************

    double lowpr = 0.35; //нижний порог
    double highpr = 0.58; //верхний порог
    double down = lowpr * 255;
    double up = highpr * 255;
    queue< pair<int, int> > nodes;

    for (i = 1; i < height_ - 1; i++)
    {
        for (j = 1; j < width_ - 1; j++)
        {
            LightMap2[i][j] = 0;
        }
    }

    for (i = 1; i < height_ - 1; i++)
        for (j = 1; j < width_ - 1; j++) 
        {
            if((LightMap[i][j] >= up) && LightMap2[i][j] != 255) 
            {
                nodes.push(pair<int, int>(i,j));
                while(!nodes.empty()) 
                {
                    pair<int, int> node = nodes.front();
                    nodes.pop();
                    int x = node.first, y = node.second;

                    if(x < 0 || x >= (int)(height_ - 1) || y < 0 || y >= (int)(width_ - 1)) continue;

                    if(LightMap[x][y] < down) 
                        continue;

                    if(LightMap2[x][y] != 255) 
                    {
                        LightMap2[x][y] = 255;
                        nodes.push(pair<int, int>(x+1,y-1));
                        nodes.push(pair<int, int>(x+1,y  ));
                        nodes.push(pair<int, int>(x+1,y+1));
                        nodes.push(pair<int, int>(x  ,y+1));
                        nodes.push(pair<int, int>(x  ,y-1));
                        nodes.push(pair<int, int>(x-1,y-1));
                        nodes.push(pair<int, int>(x-1,y  ));
                        nodes.push(pair<int, int>(x-1,y+1));
                    }
                }
            }
        }


// *******************************************
//      запись в лайтмап того что получилось
//********************************************

    for (i = 1; i < height_ - 1; i++)
    {
        for(j = 1; j < width_ - 1; j++)
        {
            LightMap[i][j] = LightMap2[i][j];
        }
    }
}


void LightDown(Pixel** pa, long width, long height)
{
    for (int i = 0; i < height; i++) 
    {
        for (int j = 0; j < width; j++) 
        {
            pa[i][j].red = CheckBounds(pa[i][j].red - 100);
            pa[i][j].green = CheckBounds(pa[i][j].green - 100);
            pa[i][j].blue = CheckBounds(pa[i][j].blue - 100);
        }
    }
}

void ContrastAl(Pixel** pa, long width, long height)
{
    const int contrast = 90; // Сила контраста
    unsigned char buf[256];
    unsigned int midBright = 0;
    
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++) 
        {
            midBright += getLight(pa[i][j]);
        }
    
    midBright /= (width * height);
    
    double k = 1.0 + contrast / 100.0;
    
    for (int i = 0; i < 256; i++)
    {
        int delta = (int)i - midBright;
        int temp  = (int)(midBright + k *delta);
        
        if (temp < 0)
            temp = 0;
        
        if (temp >= 255)
            temp = 255;
        buf[i] = (unsigned char)temp;
        /*
         int a = (((i - midBright) * contrast) >> 8) + midBright;
         if (a < 0) buf[i] = 0;
         else if (a > 255) buf[i] = 255;
         else buf[i] = a;*/
    }
    
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++) 
        {
            pa[i][j].red = buf[pa[i][j].red];
            pa[i][j].green = buf[pa[i][j].green];
            pa[i][j].blue = buf[pa[i][j].blue];
        }
}

void NoneAl(Pixel** pa, long width, long height)
{
    // Проверить типы переменных, могут не подходить

    uint8_t Lumin = 0;
    Pixel color[9];
    uint8_t lR, lG, lB;
    long wWidth, wHeight;
    
    wWidth = width - 1;
    wHeight = height - 1;
    
    for (long i = 1; i < wHeight; i++)
    {
        for (long j = 1; j < wWidth; j++)
        {
            color[0] = pa[i][j];
            color[1] = pa[i][j-1];
            color[2] = pa[i+1][j-1];
            color[3] = pa[i+1][j];
            color[4] = pa[i+1][j+1];
            color[5] = pa[i][j+1];
            color[6] = pa[i-1][j+1];
            color[7] = pa[i-1][j];
            color[8] = pa[i-1][j-1];
            
            lR = -1 * color[1].red - 1 * color[2].red - 1 * color[3].red - 1 * color[4].red
            - 1 * color[5].red - 1 * color[6].red - 1 * color[7].red - 1 * color[8].red
            + (8 * color[0].red) / 1 + 255;
            lG = -1 * color[1].green - 1 * color[2].green - 1 * color[3].green - 1 * color[4].green
            - 1 * color[5].green - 1 * color[6].green - 1 * color[7].green - 1 * color[8].green
            + (8 * color[0].green) / 1 + 255;
            lB = -1 * color[1].blue - 1 * color[2].blue - 1 * color[3].blue - 1 * color[4].blue
            - 1 * color[5].blue - 1 * color[6].blue - 1 * color[7].blue - 1 * color[8].blue
            + (8 * color[0].blue) / 1 + 255;
            
            lR = CheckBounds(lR);
            lG = CheckBounds(lG);
            lB = CheckBounds(lB);
            
            //Lumin = Gray(lR, lG, lB);
            
            if (Lumin > 160) 
            {
                pa[i][j].red = 0;
                pa[i][j].green = 0;
                pa[i][j].blue = 0;
            }
            else
            {
                pa[i][j].red = 255;
                pa[i][j].green = 255;
                pa[i][j].blue = 255;
            }
        }
    }
}

void SobelAl(Pixel** pa, long width, long height)
{
    uint8_t sum = 0;
    int x, y;
    int i, j;
    
    Pixel** result;
    
    result = new Pixel*[height];
    
    for (i = 0; i < height; i++)
    {
        result[i] = new Pixel[width];
    }
    
    for (i = 1; i < height - 1; i++)
    {
        for (j = 1; j < width - 1; j++) 
        {
            x = (getLight(pa[i-1][j+1]) + 2 * getLight(pa[i][j+1]) + getLight(pa[i+1][j+1])) - (getLight(pa[i-1][j-1]) + 2 * getLight(pa[i][j-1]) + getLight(pa[i+1][j-1]));
            
            y = (getLight(pa[i-1][j-1]) + 2 * getLight(pa[i-1][j]) + getLight(pa[i-1][j+1])) - (getLight(pa[i+1][j-1]) + 2 * getLight(pa[i+1][j]) + getLight(pa[i+1][j+1]));
            
            sum = CheckBounds(round(sqrt((double)(x * x + y * y))));
            
            //sum = CheckBounds(sum);
            setPixel(result[i][j], sum);
        }
    }
    
    for (i = 1; i < height - 1; i++)
    {
        for (j = 1; j < width - 1; j++) 
        {
            pa[i][j] = result[i][j];
        }
    }
    
    for (i = 0; i < height; i++) 
    {
        delete[] result[i];
    }
    delete[] result;
}

void SobelAl(int** pa, double** dX, double** dY, double** G, int height, int width)
{
    int x, y;
    int i, j;

    for (i = 1; i < height - 1; i++)
    {
        for (j = 1; j < width - 1; j++) 
        {
            x = (pa[i-1][j+1] + 1 * pa[i][j+1] + pa[i+1][j+1]) - (pa[i-1][j-1] + 1 * pa[i][j-1] + pa[i+1][j-1]);
            
            y = (pa[i-1][j-1] + 1 * pa[i-1][j] + pa[i-1][j+1]) - (pa[i+1][j-1] + 1 * pa[i+1][j] + pa[i+1][j+1]);
            
            dX[i][j] = x;
            dY[i][j] = y;
            G[i][j] = sqrt((double)(x * x + y * y));
        }
    }
}

void RobertsAl(Pixel** pa, long width, long height)
{
    int RT, LT, RB, LB;
    int ResLum;
    uint8_t ResLum1;
    
    for (int i = 0; i < height - 1; i++) 
    {
        for (int j = 0; j < width - 1; j++) 
        {
            LT = getLight(pa[i][j]);
            RT = getLight(pa[i][j+1]);
            LB = getLight(pa[i+1][j]);
            RB = getLight(pa[i+1][j+1]);
            ResLum = round(sqrt((double)((LT - RB)*(LT - RB)+(LB - RT)*(LB - RT))));
            ResLum1 = CheckBounds(ResLum);
            //ResLum = 255 - ResLum;
            pa[i][j].red = ResLum;
            pa[i][j].green = ResLum;
            pa[i][j].blue = ResLum;
        }
    }
    for (int i = 0; i < height - 1; i++) 
    {
        for (int j = 0; j < width - 1; j++) 
        {
            if (pa[i][j].red != 0)
            {
                pa[i][j].red = 255;
                pa[i][j].green = 255;
                pa[i][j].blue = 255;
            }
        }
    }
}

void MedianAl(Pixel** pa, long width, long height)
{
    int color[9];
    
    for (long i = 1; i < height - 1; i++)
    {
        for (long j = 1; j < width - 1; j++)
        {
            color[0] = getLight(pa[i][j]);
            color[1] = getLight(pa[i][j-1]);
            color[2] = getLight(pa[i+1][j-1]);
            color[3] = getLight(pa[i+1][j]);
            color[4] = getLight(pa[i+1][j+1]);
            color[5] = getLight(pa[i][j+1]);
            color[6] = getLight(pa[i-1][j+1]);
            color[7] = getLight(pa[i-1][j]);
            color[8] = getLight(pa[i-1][j-1]);
            QSort(color, 0, 8);
            
            pa[i][j].red = color[4];
            pa[i][j].green = color[4];
            pa[i][j].blue = color[4];
        }
    }
}

void MedianAl(int** pa, long width, long height)
{
    int color[9];
    
    for (long i = 1; i < height - 1; i++)
    {
        for (long j = 1; j < width - 1; j++)
        {
            color[0] = pa[i][j];
            color[1] = pa[i][j-1];
            color[2] = pa[i+1][j-1];
            color[3] = pa[i+1][j];
            color[4] = pa[i+1][j+1];
            color[5] = pa[i][j+1];
            color[6] = pa[i-1][j+1];
            color[7] = pa[i-1][j];
            color[8] = pa[i-1][j-1];
            QSort(color, 0, 8);
            
            pa[i][j] = color[4];
        }
    }
}

void QSort(int* color, long low, long high)
{
    long i = low;
    long j = high;
    int temp;
    int x = color[(low+high)/2];
    do 
    {
        while(color[i] < x) ++i;
        while(color[j] > x) --j;
        if (i <= j)
        {
            temp = color[i];
            color[i] = color[j];
            color[j] = temp;
            i++; j--;
        }
    } while (i <= j);
    
    if (low < j) 
        QSort(color, low, j);
    if (i < high) 
        QSort(color, i, high);
}

void GrayScale(Pixel** pa, int height, int width)
{
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++) 
        {
            setPixel(pa[i][j], getLight(pa[i][j]));
        }
    }
}

void Laplas(Pixel** pa, long width, long height)
{
    int i, j;
    int x;
    Pixel** result;
    
    result = new Pixel*[height];
    
    for (i = 0; i < height; i++)
    {
        result[i] = new Pixel[width];
    }
    
    for (i = 1; i < height - 1; i++)
    {
        for (j = 1; j < width - 1; j++) 
        {
            x = getLight(pa[i-1][j]) + getLight(pa[i][j-1]) - 4 * getLight(pa[i][j]) + getLight(pa[i][j+1]) + getLight(pa[i+1][j]);
            
            x = CheckBounds(x);
            setPixel(result[i][j], x);
        }
    }
    
    for (i = 1; i < height - 1; i++)
    {
        for (j = 1; j < width - 1; j++) 
        {
            pa[i][j] = result[i][j];
        }
    }
    
    for (i = 0; i < height; i++) 
    {
        delete[] result[i];
    }
    delete[] result;
}

void GaussKernel(float sigma, int  masksize, float** kernel)
{
    int i;
    float coeff = 1 / (2.0f * (float)M_PI * sigma * sigma);
    
    for (i = -masksize / 2; i <= masksize / 2; i++)
    {
        for (int j = -masksize / 2; j <= masksize / 2; j++)
        {
            kernel[i + masksize / 2][j + masksize / 2] = coeff * (float)exp(-(float)(i * i + j * j) / (2 * sigma * sigma));
        }
    }
}

void GaussianBlurring(int** pa, long width, long height)
{
    float lR = 0;
    int mask = 3, i, j, cmask;
    float sigma = 1.0; 
    
    float** kernel;
    
    kernel = new float*[mask];
    for (i = 0; i < mask; i++) 
    {
        kernel[i] = new float[mask];
    }
    
    GaussKernel(sigma, mask, kernel);
    
    cmask = mask / 2 + 1;
    
    for (i = cmask; i < height - cmask; i++) 
    {
        for (j = cmask; j < width - cmask; j++) 
        {
            for (int k = 0; k < mask; k++) 
            {
                for (int m = 0; m < mask; m++) 
                {
                    lR += pa[i - cmask + k + 1][j - cmask + m + 1] * kernel[k][m];
                }
            }
            pa[i][j] = round(lR);
            lR = 0;
        }
    }
    
    for (i = 0; i < mask; i++) 
    {
        delete[] kernel[i];
    }
    delete[] kernel;
}


/*
 
 Name: CheckBounds 
 Description: Если компонент пикселя достаточно яркий, то делаем его белым.
 Если достаточно тёмный, то делаем черным. В остальных случаях оставляем высчитанный цвет.
 
*/

uint8_t CheckBounds(int tValue)
{
    if (tValue < 0) tValue = 0;
    if (tValue > 250) tValue = 255;
    return tValue;
}