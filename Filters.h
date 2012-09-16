#ifndef Limin_Filters_h
#define Limin_Filters_h

#include <vector>
#include <queue>
#include "BMP.h"
#include "Base.h"

using namespace std;

void Canny(int** LightMap, long width, long height);
void LightDown(Pixel** pa, long width, long height);
void ContrastAl(Pixel** pa, long width, long height);
void SobelAl(Pixel** pa, long width, long height);
void NoneAl(Pixel** pa, long width, long height);
void GrayScale(Pixel** pa);
uint8_t** GrayScale();
void Laplas(Pixel** pa, long width, long height);
void GaussianBlurring(int** pa, long width, long height);
void RobertsAl(Pixel** pa, long width, long height);
void MedianAl(Pixel** pa, long width, long height);
void MedianAl(int** pa, long width, long height);
void QSort(int* color, long low, long high);
void GaussKernel(float sigma, int masksize, float** kernel);
void SobelAl(int** pa, double** dX, double** dY, double** G);
uint8_t CheckBounds(int tValue);

#endif