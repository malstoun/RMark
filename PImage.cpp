#define _USE_MATH_DEFINES
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <queue>
#include "PImage.h"
#include "IOBMP.h"
#include "Filters.h"

PImage::PImage(BMPImage img)
{
    if (img.getWidth() > 640)
    {
        this->width_ = 640;
        this->height_ = width_ * img.getHeight() / img.getWidth();
        image_ = Interpolation(img);
    }
	else
	{
		this->width_ = img.getWidth();
		this->height_ = img.getHeight();
		image_ = img;
	}
}

/*
 
 Name: transBaW
 Description: Перевод в "чёрное-белое" изображение годное для распознавания и дальнейшего улучшения
 
*/

bool PImage::transBaW()
{
    Pixel** PixelArray; // Массив пикселей, с которым будем работать
	int** LightMap;
	uint32_t i, j, ii, jj; // Переменные циклов
    float a, b;
    float py, pe;
    float alpha1, alpha2;
	int sum = 0;
	int max1 = 0, max2 = 0, max3 = 0, max4 = 0;
	int countRed = 0;
	int countWhite = 0;
	int x, y, last;
    int peakRegX[2];
    int peakRegY[2];
    uint32_t wReg = 0, hReg = 0;
    uint32_t bWidthReg = 0, bHeightReg = 0;
    widthTile_ = width_ / 3; // Максимальная длина пластины
    heightTile_ = height_ / 10; // Максимальная высота пластины
	vector<int> counterRed;
	vector<int> counterWhite;
    vector<int*> Accums;
    vector<vector<ExtPeak> > Vert;
    vector<ExtPeak> Paral;
    vector<Point> Peaks; // Максимальные перепады яркостей
    Point pt;
    bool flag = true;
    peakRegX[0] = 0;
    peakRegX[1] = 0;
    peakRegY[0] = 0;
    peakRegY[1] = 0;

	LightMap = new int*[height_];
    PixelArray = new Pixel*[height_];
    for (i = 0; i < height_; i++) 
    {
		LightMap[i] = new int[width_];
        PixelArray[i] = new Pixel[width_];
        for (j = 0; j < width_; j++) 
        {
			PixelArray[i][j] = image_.getPixel(i, j); // Копируем исходное изображение в "рабочий"
        }
    }
    
    ContrastAl(PixelArray, width_, height_); // Увеличиваем контраст
    for (i = 0; i < height_; i++)
    {
        for (j = 0; j < width_; j++)
        {
            LightMap[i][j] = getLight(PixelArray[i][j]);
        }
    }
    GaussianBlurring(LightMap, width_, height_);

    //Canny(PixelArray, LightMap, width_, height_); // Выделяем границы
    
    for (i = 5; i < height_ - 5 - heightTile_; i += heightTile_)
    {
        for (j = 5; j < width_ - 5 - widthTile_; j += widthTile_)
        {
            for (ii = 0; ii < heightTile_; ii++)
            {
                for (jj = 0; jj < widthTile_; jj++)
                {
                    sum += abs(LightMap[i + ii][j + jj] - LightMap[i + ii][j + jj - 1]);
                }
                if (sum > max1)
                {
                    max1 = sum;
                    pt.i_ = i;
                    pt.j_ = j;
                    pt.weight_ = sum;
                    pt.maxi_ = i + ii;
                }
                sum = 0;
            }
            max1 = 0;
            Peaks.push_back(pt);
        }
    }
    
    for (i = 0; i < Peaks.size(); i++)
    {
        if (Peaks[i].weight_ > max1)
        {
            max2 = i;
            max1 = Peaks[i].weight_;
        }
    }
    
    uint32_t begi, begj, endi, endj;
    
    begi = (int)(Peaks[max2].maxi_ - heightTile_) < 0 ? 5 : Peaks[max2].maxi_ - heightTile_;
    
    endi = (Peaks[max2].maxi_ + heightTile_) > height_ ? height_ - 5 : Peaks[max2].maxi_ + heightTile_;
    
    begj = (int)(Peaks[max2].j_ - widthTile_ / 3) < 0 ? 5 : (Peaks[max2].j_ - widthTile_ / 3);
    
    endj = (Peaks[max2].j_ + widthTile_ + widthTile_ / 3) > width_ ? width_ - 5 : (Peaks[max2].j_ + widthTile_ + widthTile_ / 3);
    
    
    MedianAl(LightMap, width_, height_);
    
    regImage_.Init(endj - begj, endi - begi);

    for (i = begi; i < endi; i++)
    {
        for (j = begj; j < endj; j++)
        {
            regImage_.setPixel(i - begi, j - begj, LightMap[i][j]);
        }
    }

    //******************** ТЕСТОВАЯ СЕКЦИЯ *************************//
    

    BMPImage histogramV; // Изображение, в которое забьём вертикальную гистограмму средней яркости по высоте
    histogramV.Init(endj - begj, 255);
    sum = 0;
    max1 = 0;
    max3 = 0;
    last = -10000;

    for (j = begj; j < endj; j++)
    {
        for (i = begi; i < endi; i++)
        {
            sum += abs(LightMap[i][j] - LightMap[i][j+1]); // Считаем сумму перепадов
        }
        for (x = 0; x < 255; x++)
        {
            if (x < (sum / (endi - begi)))
            {
                histogramV.setPixel(x, j - begj, 0);
            }
            else
            {
                histogramV.setPixel(x, j - begj, 255);
            }
        }
        if (sum > max3) // Ищем первый максимум в гистограмме
        {
            max3 = sum;
            peakRegX[0] = j;
            last = j;
        }
        sum = 0;
    }
    sum = 0;
    max3 = 0;
    for (j = begj; j < endj; j++) // Пришлось выделить в отдельный цикл поиск второго максимума в гистограмме
    {
        for (i = begi; i < endi; i++)
        {
            sum += abs(LightMap[i][j] - LightMap[i][j+1]);
        }
        if (sum > max3 && abs(last - (int)j) > 10)
        {
            max3 = sum;
            peakRegX[1] = j;
        }
        sum = 0;
    }
    
    
    IOBMP wr;
    wr.setBmp(histogramV, "hist");
    
    histogramV.Init(endi - begi, 255); // Аналогично выполняем для горизонтали
    sum = 0;
    max2 = 0;
    max4 = 0;
    last = -10000;
    
    for (i = begi; i < endi; i++)
    {
        for (j = begj; j < endj; j++)
        {
            sum += abs(LightMap[i][j] - LightMap[i+1][j]);
        }
        for (x = 0; x < 255; x++)
        {
            if (x < (sum / (endj - begj)))
            {
                histogramV.setPixel(x, i - begi, 0);
            }
            else
            {
                histogramV.setPixel(x, i - begi, 255);
            }
        }
        if (sum > max4)
        {
            max4 = sum;
            peakRegY[0] = i;
            last = i;
        }
        sum = 0;
    }
    sum = 0;
    max4 = 0;
    for (i = begi; i < endi; i++)
    {
        for (j = begj; j < endj; j++)
        {
            sum += abs(LightMap[i][j] - LightMap[i-1][j]);
        }
        if (sum > max4 && abs(last - (int)i) > 8)
        {
            max4 = sum;
            peakRegY[1] = i;
        }
        sum = 0;
    }

    wr.setBmp(histogramV, "histH");
    
    if (peakRegX[0] < peakRegX[1])
    {
        swap(peakRegX[0], peakRegX[1]);
    }
    if (peakRegY[0] < peakRegY[1])
    {
        swap(peakRegY[0], peakRegY[1]);
    }
	loc_regImage_.Init(peakRegX[0] - peakRegX[1], peakRegY[0] - peakRegY[1]);
    for (i = peakRegY[1]; i < peakRegY[0]; i++)
    {
        for (j = peakRegX[1]; j < peakRegX[0]; j++)
        {
			loc_regImage_.setPixel(i - peakRegY[1], j - peakRegX[1], LightMap[i][j]);
        }
    }
    
    //***************** КОНЕЦ ТЕСТОВОЙ СЕКЦИИ **********************//
    
    
	for	(i = 0; i < height_; i++)
	{
		for	(j = 0; j < width_; j++)
		{
			setPixel(PixelArray[i][j], LightMap[i][j]); // Копируем из LightMap в рабочий PixelArray
			setPixel(PixelArray[0][j], 0); // Зануляем края
			setPixel(PixelArray[1][j], 0);
			setPixel(PixelArray[2][j], 0);
            setPixel(PixelArray[3][j], 0);
			setPixel(PixelArray[height_ - 1][j], 0);
			setPixel(PixelArray[height_ - 2][j], 0);
			setPixel(PixelArray[height_ - 3][j], 0);
            setPixel(PixelArray[height_ - 4][j], 0);
		}
		setPixel(PixelArray[i][0], 0);
		setPixel(PixelArray[i][1], 0);
		setPixel(PixelArray[i][2], 0);
        setPixel(PixelArray[i][3], 0);
		setPixel(PixelArray[i][width_ - 1], 0);
		setPixel(PixelArray[i][width_ - 2], 0);
		setPixel(PixelArray[i][width_ - 3], 0);
        setPixel(PixelArray[i][width_ - 4], 0);
	}

    for (i = 0; i < 0; i++) // Тут исправить 0 на 9!!!!!
    {
        for (int j = 0; j < 2; j++) 
        {
            Vert.push_back(DetectRect(CalcAccum(PixelArray, i * heightTile_, j * widthTile_), i * heightTile_, j * widthTile_));
        }
    }
    
    
    for (i = 0; i < Vert.size(); i++) 
    {
        for (j = 0; j < Vert[i].size(); j += 2) 
        {
            alpha1 = fabs(Vert[i][j].BAngle - Vert[i][j+1].BAngle);
            alpha1 = rad(alpha1);
            a = fabs(Vert[i][j].Ep1 - Vert[i][j].Ep2 / sinf(alpha1));
            b = fabs(Vert[i][j+1].Ep1 - Vert[i][j+1].Ep2 / sinf(alpha1));
            pe = (float)(2. * (a + b));
            py = (float)(2. * (abs(Vert[i][j].Ep1 - Vert[i][j].Ep2) + abs(Vert[i][j+1].Ep1 - Vert[i][j+1].Ep2)));

            if (fabs(py - pe) < 0.1 * pe) // Поменял с 0.2 на 0.1
            {
                /*
                    В if можно вставить функцию, в которой определяется отношение сторон и 
                    принадлежность к номеру какой-либо страны
                    Сейчас ищем прямоугольник, если убрать будет искать всё подряд => дольше, но больше вариантов
                */
                if ((a / b) >= 3 && (a / b) <= 6)
                {
                    Paral.push_back(Vert[i][j]);
                    Paral.push_back(Vert[i][j+1]);
                }
            }
        }
    }
    
	if (Paral.size() > 0)
    {
		for (i = 0; i < Paral.size(); i += 2)
        {
			if (Paral[i].Ep1 > Paral[i].Ep2)
			{
				max1 = Paral[i].Ep1;
				max2 = Paral[i].Ep2;
			}
			else
			{
				max1 = Paral[i].Ep2;
				max2 = Paral[i].Ep1;
			}

			if (Paral[i+1].Ep1 > Paral[i+1].Ep2)
			{
				max3 = Paral[i+1].Ep1;
				max4 = Paral[i+1].Ep2;
			}
			else
			{
				max3 = Paral[i+1].Ep2;
				max4 = Paral[i+1].Ep1;
			}

            alpha1 = Paral[i].BAngle;
            alpha2 = Paral[i+1].BAngle;
			
			for(j = 0; j < whitePt.size(); j++)
            {
				y = whitePt[j].i_;
				x = whitePt[j].j_;
                if (round((y * sin(rad(alpha1)) + x * cos(rad(alpha1)))) <= max1)
                {
					if (round((y * sin(rad(alpha1)) + x * cos(rad(alpha1)))) >= max2)
					{
						if (round((y * sin(rad(alpha2)) + x * cos(rad(alpha2)))) <= max3)
						{
							if (round((y * sin(rad(alpha2)) + x * cos(rad(alpha2)))) >= max4)
                            {
								countRed++;
							}
						}
					}
                }
            }

			counterRed.push_back(countRed);
			countRed = 0;
        }
    }
    
	countRed = -1;
	sum = 0;

	for (i = 0; i < counterRed.size(); i++)
		if (countRed < counterRed[i])
		{
			sum = i;
			countRed = counterRed[i];
		}

	sum *= 2;
    
	if (Paral.size() > 0)
	{
		if (Paral[sum].Ep1 > Paral[sum].Ep2)
		{
			max1 = Paral[sum].Ep1;
			max2 = Paral[sum].Ep2;
		}
		else
		{
			max1 = Paral[sum].Ep2;
			max2 = Paral[sum].Ep1;
		}

		if (Paral[sum+1].Ep1 > Paral[sum+1].Ep2)
		{
			max3 = Paral[sum+1].Ep1;
			max4 = Paral[sum+1].Ep2;
		}
		else
		{
			max3 = Paral[sum+1].Ep2;
			max4 = Paral[sum+1].Ep1;
		}
        
        // ИЗМЕНЕНИЕ!
        
		alpha1 = Paral[sum].BAngle;
		alpha2 = Paral[sum+1].BAngle;

        wReg = (max3 - max4) * fabsf(sinf(rad(alpha1))) + (max1 - max2) * fabsf(cosf(rad(alpha1)));
        hReg = (max1 - max2) * fabsf(sinf(rad(alpha1))) + (max3 - max4) * fabsf(cosf(rad(alpha1)));
        
        regImage_.Init(wReg, hReg);
        
        bWidthReg = max4 * cosf(rad(alpha2)) + max2 * cosf(rad(alpha1));
        bHeightReg = max2 * sinf(rad(alpha1)) + max3 * sinf(rad(alpha2));
		for(uint32_t y = bHeightReg - hReg; y < bHeightReg; y++)
		{
			for(uint32_t x = bWidthReg; x < wReg + bWidthReg; x++)
			{
                regImage_.setPixel(y - bHeightReg + hReg, x - bWidthReg, image_.getLight(y, x));
			}
		}
        
        /*for(uint32_t y = 0; y < height_; y++)
		{
			for(uint32_t x = 0; x < width_; x++)
			{
                if (round((y * sin(rad(alpha1)) + x * cos(rad(alpha1)))) <= max1)
                {
                    if (round((y * sin(rad(alpha1)) + x * cos(rad(alpha1)))) >= max2)
                    {
                        if (round((y * sin(rad(alpha2)) + x * cos(rad(alpha2)))) <= max3)
                        {
                            if (round((y * sin(rad(alpha2)) + x * cos(rad(alpha2)))) >= max4)
                            {
                                PixelArray[y][x].red = 254;
                                PixelArray[y][x].green = 0;
                                PixelArray[y][x].blue = 0;
                            }
                        }
                    }
                }
			}
		}*/
	}
    
    Paral.clear();
    
    for (i = 0; i < height_; i++) 
    {
        for (j = 0; j < width_; j++) 
        {
			image_.setPixel(i, j, PixelArray[i][j].red, PixelArray[i][j].green, PixelArray[i][j].blue);
        }
    }
    
	for (i = 0; i < height_; i++)
    {
		delete[] PixelArray[i];
    }
    delete[] PixelArray;
    
	return true;
}

/*
 
 Name: HoughLine
 Description: Получаем аккумулятор
 
*/

int* PImage::CalcAccum(Pixel** pa, long heightB, long widthB)
{
    int i, j;
    long height, width;
    int k;
    uint32_t RMax;
    int* PhaseSpace;
    
	height = heightB + 2 * heightTile_;
	width = widthB + 2 * widthTile_;
    
    RMax = round(sqrt((double)(width * width + height * height)));

	PhaseSpace = (int*) malloc(sizeof(int) * (angle + 30) * RMax);
    
    for (i = -29; i < angle; i++)
        for (j = 0; j < RMax; j++)
            PhaseSpace[(i+29) * RMax + j] = 0;

        
    for (long n = heightB; n < height; n++)
    {
        for (long m = widthB; m < width; m++)
        {
            if (isWhite(pa[n][m]))
            {
                for (i = -29; i < angle; i++) 
                {
                    k = abs(round((float)m * cosf(rad(i)) + (float)n * sinf(rad(i))));
                    PhaseSpace[(i+29) * RMax + k]++; // Здесь в mac надо менять на fabs, в win - abs
                }
            }
        }
    }
    
    return PhaseSpace;
}

vector<ExtPeak> PImage::DetectRect(int* PhaseSpace, long heightB, long widthB)
{
    int TAngle = 3; // 5
    short unsigned int switcher = 0;
    float TWeight = (float)0.35; // .55
    int i, j, k;
    long width, height;
    float maxP;
    int MaxPhaseValue = 0;
    bool flag = false;
    uint32_t RMax;
    float alphakl;
    float cond1, cond2;
    PhasePoint temp[4];
    ExtPeak temp4;
    vector<PhasePoint> Peaks;
    vector<ExtPeak> ExtPeaks;
    vector<ExtPeak> ResPeaks;
    int minHeight = round(height_ / 24);
    int minWidth = round(3 * minHeight);
    
    height = heightB + 2 * heightTile_;
	width = widthB + 2 * widthTile_;
    
    RMax = round(sqrt((double)(width * width + height * height)));

    for (i = -29; i <= 30; i++) 
    {
        for (j = 0; j < RMax; j++)
        {
            if (PhaseSpace[(i+29) * RMax + j] > MaxPhaseValue && PhaseSpace[(i+29) * RMax + j] > minHeight)                
            {
                MaxPhaseValue = PhaseSpace[(i+29) * RMax + j];
                //temp[3] = temp[2];
                //temp[2] = temp[1];
                temp[1] = temp[0];
                temp[0].set(i, j, MaxPhaseValue);
                PhaseSpace[(i+29) * RMax + j] = 0;
                switcher++;
                flag = true;
            }
        }
        if (flag) 
        {
            for (k = 0; k < switcher && k < 2/*4*/; k++)
                Peaks.push_back(temp[k]);
                
            switcher = 0;
            flag = false;
            MaxPhaseValue = 0;
        }
    }
    
    for (i = 60; i <= 120; i++) 
    {
        for (j = 0; j < RMax; j++)
        {
            if (PhaseSpace[(i+29) * RMax + j] > MaxPhaseValue && PhaseSpace[(i+29) * RMax + j] > minWidth)                
            {
                MaxPhaseValue = PhaseSpace[(i+29) * RMax + j];
                //temp[3] = temp[2];
                //temp[2] = temp[1];
                temp[1] = temp[0];
                temp[0].set(i, j, MaxPhaseValue);
                PhaseSpace[(i+29) * RMax + j] = 0;
                switcher++;
                flag = true;
            }
        }
        if (flag) 
        {
            for (k = 0; k < switcher && k < 2/*4*/; k++)
                Peaks.push_back(temp[k]);
                
            switcher = 0;
            flag = false;
            MaxPhaseValue = 0;
        }
    }
    
    for (i = 150; i < 180; i++) 
    {
        for (j = 0; j < RMax; j++)
        {
            if (PhaseSpace[(i+29) * RMax + j] > MaxPhaseValue && PhaseSpace[(i+29) * RMax + j] > minHeight)                
            {
                MaxPhaseValue = PhaseSpace[(i+29) * RMax + j];
                //temp[3] = temp[2];
                //temp[2] = temp[1];
                temp[1] = temp[0];
                temp[0].set(i, j, MaxPhaseValue);
                PhaseSpace[(i+29) * RMax + j] = 0;
                switcher++;
                flag = true;
            }
        }
        if (flag) 
        {
            for (k = 0; k < switcher && k < 2/*4*/; k++)
                Peaks.push_back(temp[k]);
            
            switcher = 0;
            flag = false;
            MaxPhaseValue = 0;
        }
    }
    
    for (i = 0; i < Peaks.size(); i++) 
    {
        for (j = i + 1; j < Peaks.size(); j++) 
        {
            if (abs(Peaks[i].getTeta() - Peaks[j].getTeta()) < TAngle)
            {
                if (abs(Peaks[i].getVote() - Peaks[j].getVote()) < (TWeight * (Peaks[i].getVote() + Peaks[j].getVote()) / 2))
                {
                    temp4.BAngle = (float)(0.5 * (Peaks[i].getTeta() + Peaks[j].getTeta()));
                    temp4.Ep1 = Peaks[i].getR();
                    temp4.Ep2 = Peaks[j].getR();
                    temp4.Vote = round((Peaks[i].getVote() + Peaks[j].getVote()) / 2);
                    ExtPeaks.push_back(temp4);
                }
            }
            else
            {
                break;
            }
        }
    }
    
    for (i = 0; i < ExtPeaks.size(); i++) 
    {
        for (j = i + 1; j < ExtPeaks.size(); j++) 
        {
            alphakl = fabs(ExtPeaks[i].BAngle - ExtPeaks[j].BAngle);
            if (alphakl > 85 && alphakl < 95)
            {
                alphakl = rad(alphakl); // Новая!
                cond1 = fabs((abs(ExtPeaks[i].Ep1 - ExtPeaks[i].Ep2) - (float)(ExtPeaks[j].Vote) * sinf(alphakl)) / (abs(ExtPeaks[i].Ep1 - ExtPeaks[i].Ep2)));
                cond2 = fabs((abs(ExtPeaks[j].Ep1 - ExtPeaks[j].Ep2) - (float)(ExtPeaks[i].Vote) * sinf(alphakl)) / (abs(ExtPeaks[j].Ep1 - ExtPeaks[j].Ep2)));
                maxP = max(cond1, cond2);
                if (maxP < TWeight) 
                {
                    ResPeaks.push_back(ExtPeaks[i]);
                    ResPeaks.push_back(ExtPeaks[j]);
                }
            }
        }
    }
    
    Peaks.clear();
    ExtPeaks.clear();
    return ResPeaks;
}



BMPImage PImage::getRegImage()
{
    return regImage_;
}

BMPImage PImage::getLocRegImage()
{
	return loc_regImage_;
}

BMPImage PImage::Interpolation(BMPImage src)
{  
	BMPImage dest;
	int newW = 640, newH = newW * src.getHeight() / src.getWidth();
	int srcH = src.getHeight(), srcW = src.getWidth();
	dest.Init(newW, newH);
	//process each line
	for (int iRow = 0; iRow < newH; iRow++)
	{
		double ay = (double) iRow * srcH / newH;    //scaled source iRow
		int     y = (int)ay;                                    //truncates source iRow
		ay = ay - y;                                        //distance as fractional part of the scaled pixel, used for weighting

		//process each pixel within the line
		for (int iCol = 0; iCol < newW; iCol++)
		{
			double R, G, B;

			double ax = (double)iCol * srcW / newW;    //scaled source column
			int     x = (int)ax;                                //truncates source column
			ax = ax - x;                                    //distance as fractional part of the scaled pixel, used for weighting

			if ((x < srcW - 1) && (y < srcH - 1))
			{
				//interpolate each channel
				R = (1.0 - ax) * (1.0 - ay) * src.getPixel(y+0, x+0).red + 
				ax  * (1.0 - ay) * src.getPixel(y+0, x+1).red +
				(1.0 - ax) * ay  * src.getPixel(y+1, x+0).red +
				ax  *        ay  * src.getPixel(y+1, x+1).red;
 
				G = (1.0 - ax) * (1.0 - ay) * src.getPixel(y+0, x+0).green + 
				ax  * (1.0 - ay) * src.getPixel(y+0, x+1).green + 
				(1.0 - ax) *        ay  * src.getPixel(y+1, x+0).green +
				ax  *        ay  * src.getPixel(y+1, x+1).green;

				B = (1.0 - ax) * (1.0 - ay) * src.getPixel(y+0, x+0).blue + 
				ax  * (1.0 - ay) * src.getPixel(y+0, x+1).blue + 
				(1.0 - ax) *        ay  * src.getPixel(y+1, x+0).blue +
				ax  *        ay  * src.getPixel(y+1, x+1).blue;
			}
			else
			{
				//set to zero to blank these lines instead of copying source
				R = src.getPixel(y, x).red;
				G = src.getPixel(y, x).green;
				B = src.getPixel(y, x).blue;
			}

			//write to destination buffer, change order for BGR
			dest.setPixel(iRow, iCol, (uint8_t)R, (uint8_t)G, (uint8_t)B);
		}
	}
	return dest;
}



////////////////////////////////////////////////////////////////
//
//
//
// Элементарные преобразования изображений
//
//
////////////////////////////////////////////////////////////////


/*
 
 Name: GetIntImage
 Description: Создает интегральное представление изображения. Теперь не используется, но на всякий случай оставил
 Вполне может понадобиться.
 
 */

int** PImage::GetIntImage(Pixel** pa, long width, long height)
{
    int** result;
    
    result = (int**) malloc(height * sizeof(int*));
    
    for (long i = 0; i < height; i++)
    {
        result[i] = (int*) malloc(width * sizeof(int));
    }
    
    result[0][0] = Gray(pa[0][0].red, pa[0][0].green, pa[0][0].blue);
    
    for (long i = 1; i < height; i++)
    {
        result[i][0] = Gray(pa[i][0].red, pa[i][0].green, pa[i][0].blue);
        result[i][0] += result[i-1][0];
    }
    
    for (long j = 1; j < width; j++)
    {
        result[0][j] = Gray(pa[0][j].red, pa[0][j].green, pa[0][j].blue);
        result[0][j] += result[0][j-1];
    }
    
    
    for (long i = 1; i < height; i++)
    {
        for (long j = 1; j < width; j++)
        {
            result[i][j] = Gray(pa[i][j].red, pa[i][j].green, pa[i][j].blue);
            result[i][j] += result[i-1][j] + result[i][j - 1] - result[i - 1][j - 1];
        }
    }
    
    
    return result;
    
}

/*
 
 Name: GetLumAround
 Description: Высчитывает среднюю яркость вокруг пикселя через интегральное представление.
 Если будем использовать инт.пр, то это точно понадобится.
 
 */

int PImage::GetLumAround(int **integralImage, long width, long height, long i, long j)
{
    int A = 0, B = 0, C = 0, D = 0;
    
    long begI = (i - 1 >= 0) ? i - 1 : 0;
	long begJ = (j - 1 >= 0) ? j - 1 : 0;
	long endI = (i + 1 < height) ? i + 1 : height - 1;
	long endJ = (j + 1 < width) ? j + 1 : width - 1;
    
    A = integralImage[begI][begJ];
    B = integralImage[begI][endJ];
    C = integralImage[endI][endJ];
    D = integralImage[endI][begJ];
    
    return A + C - B - D;
}

/*
 
 Name: GetLumAroundEasy
 Description: Высчитывает среднюю яркость вокруг пикселя. Использовалось раньше в случае,
 если необходимо было обработать всего пару-тройку пикселей. 
 
 */

int PImage::GetLumAroundEasy(Pixel **pa, long width, long height, long i, long j)
{
    int Lumin = 0;
	int count = 4;
	long begI = (i - 1 >= 0) ? i - 1 : 0;
	long begJ = (j - 1 >= 0) ? j - 1 : 0;
	long endI = (i + 1 < height) ? i + 1 : height - 1;
	long endJ = (j + 1 < width) ? j + 1 : width - 1;
    
    
	Lumin += Gray(pa[begI][begJ].red, pa[begI][begJ].green, pa[begI][begJ].blue);
	Lumin += Gray(pa[begI][endJ].red, pa[begI][endJ].green, pa[begI][endJ].blue);
	Lumin += Gray(pa[endI][begJ].red, pa[endI][begJ].green, pa[endI][begJ].blue);
	Lumin += Gray(pa[endI][endJ].red, pa[endI][endJ].green, pa[endI][endJ].blue);
    
	return (Lumin / count);
}


/*void PImage::BitBinar(Pixel** pa, long width, long height)
{
    int i, j;
    char k;
    bool b[9];
    bool res[9];
    char Light = 0;
    
    for (i = 1; i < height - 1; i++) 
    {
        for (j = 1; j < width - 1; j++) 
        {
            Light = 0;
            for (k = 0; k < 9; k++) 
            {
                b[0] = (getLight(this->Image[i-1][j-1]) & (0x1 << k)) == 0 ? 0 : 1;
                b[1] = (getLight(this->Image[i-1][j]) & (0x1 << k)) == 0 ? 0 : 1;
                b[2] = (getLight(this->Image[i-1][j+1]) & (0x1 << k)) == 0 ? 0 : 1;
                b[3] = (getLight(this->Image[i][j-1]) & (0x1 << k)) == 0 ? 0 : 1;
                b[4] = (getLight(this->Image[i][j]) & (0x1 << k)) == 0 ? 0 : 1;
                b[5] = (getLight(this->Image[i][j+1]) & (0x1 << k)) == 0 ? 0 : 1;
                b[6] = (getLight(this->Image[i+1][j-1]) & (0x1 << k)) == 0 ? 0 : 1;
                b[7] = (getLight(this->Image[i+1][j]) & (0x1 << k)) == 0 ? 0 : 1;
                b[8] = (getLight(this->Image[i+1][j+1]) & (0x1 << k)) == 0 ? 0 : 1;
                res[k] = b[4] & (!b[0] | !b[1] | !b[2] | !b[3] | !b[5] | !b[6] | !b[7] | !b[8]);
            }
            for (k = 8; k >= 0; k--) 
            {
                Light |= ((0x0 | res[k]) << k);
            }
            pa[i][j].red = Light;
            pa[i][j].green = Light;
            pa[i][j].blue = Light;
        }
    }
}*/

void PImage::Cut(BMPImage image)
{
    //создание массива для перевода в чб
    int **LightMap;
    Pixel** pixelArray;
    pixelArray = (Pixel**)malloc(image.getHeight() * sizeof(Pixel*));
    LightMap = (int**)malloc(image.getHeight() * sizeof(int*));
    for (int i = 0; i < image.getHeight(); i++) 
    {
        pixelArray[i] = (Pixel*)malloc(image.getWidth() * sizeof(Pixel));
        for (int j = 0; j < image.getWidth(); j++)
        {
            pixelArray[i][j] = image.getPixel(i, j);
        }
        LightMap[i] = (int*)malloc(image.getWidth() * sizeof(int));
    }
    for (int i = 0; i < image.getHeight(); i++) 
    {
        if (i < image.getHeight())
        {
            LightMap[i] = (int*)malloc(image.getWidth() * sizeof(int));
        }
    }

    for (int i = 0; i < image.getHeight(); i++)
    {
        for (int j = 0; j < image.getWidth(); j++)
        {   
            LightMap[i][j]=image.getLight(i,j);//копируем  изображдение в градациях серого
        }
    }

    Canny(LightMap, image.getWidth(), image.getHeight()); //выделяем области
    
    int x = 0, y = 0;
    //чистим края от мусора
    for (int i = 0; i < image.getHeight(); i++)
    {
        for (int j = 0; j < image.getWidth(); j++)
        {   
            if(LightMap[0][j]==255 || LightMap[1][j]==255)
            {
                x=i;y=j;
                do
                {
                    LightMap[x][y]=0;
                    x++;
                } while(LightMap[x][y]==255);
            }
            if(LightMap[i][0]==255 || LightMap[i][1]==255)
            {
                x=i;y=j;
                do
                {
                    LightMap[x][y]=0;
                    y++;
                } while(LightMap[x][y]==255 );
            }
        }
    }

    int count=0;
    //горизонтальные полоски
    for (int i = 0; i < image.getHeight(); i++)
    {
        for (int j = 0; j < image.getWidth(); j++)
        {   
            if(LightMap[i][j]==0)
            {
                count++;
            }

        }
        if(count > image.getWidth() - 5)//5 это допуск белых пикселей для закраски
            {
                x=0;
                do
                {
                    LightMap[i][x]=150;
                    x++;
                } while(x<image.getWidth() );
            }
        count=0;
    }

    //вертикальные полоски
    bool fl = 0, fb = 0;
    for (int j = image.getWidth(); j > -1; j--)
    {
        for (int i = image.getHeight()-1; i > -1 ; i--)
        {
            if (LightMap[i][j] == 0)
                fb=1;
            
            if (LightMap[i][j] == 255)
                fl=1;
            if (fl == 1 || fb == 1 && LightMap[i][j] == 150)
                break;
        }
        fb = 0;
        if (fl == 0)
        {
            for (int i = 0; i < image.getHeight(); i++)
            {
                LightMap[i][j]=150;
            }
        }
        fl = 0;
    }

    //считаем сколько получилось символов (для создания массива)
    int n = 0;
    for (int i = image.getHeight()-1; i > -1 ; i--)
    {
        for (int j = image.getWidth(); j > -1; j--)
        {
            if (LightMap[i][j] == 0 && LightMap[i+1][j] == 150 && LightMap[i][j-1] == 150)
            {
                n++;
            }
        }
        if (n != 0)
            break;
    }

    //создаем массив для нарезки
    int*** cut = new int**[n];

    for (int i = 0; i < n; i++)
    {
        cut[i] = new int*[image.getHeight()];

        for (int j = 0; j < image.getHeight(); j++)
            cut[i][j] = new int[image.getWidth()];
    }
    for (int k = 0; k < n; k++)
    {
        for (int i = 0; i < image.getHeight(); i++)
        {
            for (int j = 0; j < image.getWidth(); j++)
            {
                cut[k][i][j]=150;
            }
        }
    }
    //режим по  полоскам
    int ii = image.getHeight() - 1, 
        jj = image.getWidth() - 1, 
        k = -1; fl = 0; count = 0;

    for (int j = image.getWidth() - 1; j > -1; j--)
    {
        ii = image.getHeight() - 1;
        for (int i = image.getHeight() - 1; i > -1 ; i--)
        {
            if (LightMap[i][j] == 0 || LightMap[i][j] == 255)
            {
                fl = 1;
                count++;
            }
            else
            {
                fl = 0;
            }
            if (count == 1 && LightMap[i][j+1] == 150)
            {   
                k++;
            }
            if (fl == 1 && LightMap[i][j-1] == 150 )
            {
                    count=0;
            }
            if (fl == 1)
            {
                    cut[k][ii][jj] = LightMap[i][j];
                    ii--;
                    if(LightMap[i-1][j] == 150)
                    {
                        break;
                    }
            }   
        }
        if(fl == 1)
            jj--;
        fl = 0;
    }
    
    //копируем чб с полосками в избражение
    for (int i = 0; i < image.getHeight(); i++)
    {
        for (int j = 0; j < image.getWidth(); j++)
        {   
            pixelArray[i][j].red = LightMap[i][j];
            pixelArray[i][j].green = LightMap[i][j];
            pixelArray[i][j].blue = LightMap[i][j];
        }
    }
    
}

//*****************************************************
//*
//*  Секция реализации PhasePoint
//*
//*****************************************************

PhasePoint::PhasePoint()
{
    Teta_ = -1;
    R_ = -1;
    Vote_ = -1;
}

void PhasePoint::set(int Teta, int R, int Vote)
{
    Teta_ = Teta;
    R_ = R;
    Vote_ = Vote;
}

int PhasePoint::getTeta()
{
    return Teta_;
}

int PhasePoint::getR()
{
    return R_;
}

int PhasePoint::getVote()
{
    return Vote_;
}

PhasePoint PhasePoint::operator=(const PhasePoint & PP)
{
    Teta_ = PP.Teta_;
    R_ = PP.R_;
    Vote_ = PP.Vote_;
    
    return PP;
}