//
//  PImage.h
//  Limin
//
//  Created by Андрей Рацеров on 27.01.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef Limin_PImage_h
#define Limin_PImage_h

#include "BMP.h"
#include "Base.h"
#include <vector>

using namespace std;
const int angle = 180;

class  PhasePoint
{
    int Teta_;
    int R_;
    int Vote_;
public:
    PhasePoint();
    void set(int Teta, int R, int Vote);
    int getTeta();
    int getR();
    int getVote();
    PhasePoint operator=(const PhasePoint&);
};

struct ExtPeak 
{
    float BAngle;
    int Ep1;
    int Ep2;
    int Vote;
};

class PImage
{
	int width_;
	int height_;
	uint32_t widthTile_, heightTile_;
    BMPImage image_;
    BMPImage regImage_;
	BMPImage loc_regImage_;
	vector<Point> whitePt;
    
public:
	PImage(BMPImage pa); // не совсем правильный конструктор. Позже надо подправить.
	bool transBaW();
	int** GetIntImage(Pixel** pa, long width, long height);
    int GetLumAround(int** integralImage, long width, long height, long i, long j);
    int GetLumAroundEasy(Pixel** pa, long width, long height, long i, long j);
    void BitBinar(Pixel** pa, long width, long height);
    void Canny(Pixel** pa, int** LightMap);
    void Canny(Pixel** pa, int** LightMap, int height, int width);
    void Cut(BMPImage image);
	BMPImage Interpolation(BMPImage src);
    BMPImage getRegImage();
	BMPImage getLocRegImage();
    
private:
    int* CalcAccum(Pixel** pa, long heightB, long widthB);
    int* Enh(int* PhaseSpace, uint32_t RMax);
    vector<ExtPeak> DetectRect(int* PhaseSpace, long heightB, long widthB);
	// ...
};

#endif