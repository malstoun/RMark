//
//  BMP.h
//  Limin
//
//  Created by Андрей Рацеров on 27.01.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef Limin_BMP_h
#define Limin_BMP_h
#include <stdint.h>
#include <iostream>
#define Gray(R, G, B) (R * 0.3 + G * 0.59 + B * 0.11);

//typedef unsigned long uint32_t;
//typedef unsigned short uint16_t;
//typedef signed long int32_t;
//typedef unsigned char uint8_t;

typedef struct tBITMAPFILEHEADER
{ 
    uint16_t    bfType; 
    uint32_t    bfSize; 
    uint16_t    bfReserved1; 
    uint16_t    bfReserved2; 
    uint32_t    bfOffBits; 
} BitMapFH;//, *PBITMAPFILEHEADER;

typedef struct tBITMAPINFOHEADER
{
    uint32_t    biSize; 
    int32_t     biWidth; 
    int32_t     biHeight; 
    uint16_t    biPlanes; 
    uint16_t    biBitCount; 
    uint32_t    biCompression; 
    uint32_t    biSizeImage; 
    int32_t     biXPelsPerMeter; 
    int32_t     biYPelsPerMeter; 
    uint32_t    biClrUsed; 
    uint32_t    biClrImportant; 
} BitMapIH;//, *PBITMAPINFOHEADER;

typedef struct tPixel
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    
    fpos_t pos;

} Pixel; 

typedef struct tPoint
{
    uint32_t i_, j_;
    uint32_t weight_;
    uint32_t maxi_;
} Point;

    


class BMPImage
{
	uint32_t width_, height_;
	Pixel** pixelArray;
    bool exist;
public:
    BMPImage()
    {
        exist = false;
    }
    ~BMPImage()
    {
        /*if (exist)
        {
            for (int i = 0; i < height_; i++)
            {
                delete [] pixelArray[i];
            }
            delete [] pixelArray;
        }*/
    }
    void Init(uint32_t width, uint32_t height)
    {
        if (exist)
        {
            for (int i = 0; i < height_; ++i)
            {
                delete [] pixelArray[i];
            }
            delete [] pixelArray;
        }
        width_ = width;
        height_ = height;
        pixelArray = new Pixel*[height_];
        for (int i = 0; i < height_; i++)
        {
            pixelArray[i] = new Pixel[width_];
        }
        exist = true;
    }
    
	void setPixel(uint32_t i, uint32_t j, uint8_t Light)
	{
		pixelArray[i][j].red = Light;
		pixelArray[i][j].green = Light;
		pixelArray[i][j].blue = Light;
	}
    
    void setPixel(uint32_t i, uint32_t j, uint8_t red, uint8_t green, uint8_t blue)
    {
        pixelArray[i][j].red = red;
		pixelArray[i][j].green = green;
		pixelArray[i][j].blue = blue;
    }
    
    void setPixelArray (Pixel** pa)
    {
        for (int i = 0; i < height_; i++)
        {
            for (int j = 0; j < width_; j++)
            {
                pixelArray[i][j] = pa[i][j];
            }
        }
    }

	uint8_t getLight(uint32_t i, uint32_t j)
	{
		return Gray(pixelArray[i][j].red, pixelArray[i][j].green, pixelArray[i][j].blue);
	}
    
    Pixel getPixel(uint32_t i, uint32_t j)
    {
        return pixelArray[i][j];
    }
    
    uint32_t getWidth()
    {
        return width_;
    }
    
    uint32_t getHeight()
    {
        return height_;
    }
    
    bool isExist()
    {
        return exist;
    }

};

#endif
