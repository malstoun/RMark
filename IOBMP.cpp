//
//  IOBMP.cpp
//  Limin
//
//  Created by Андрей Рацеров on 29.03.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//
#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include "IOBMP.h"
#include <math.h>

using namespace std;

STATUS IOBMP::getBmp(char* file_name)
{
    FILE *image;
    uint8_t r, g, b;
    unsigned long fileSize;
    unsigned long offbyte;
	unsigned short bitsp;
    unsigned long k, rowLenght;
	bool flag = false;
    
    if ((image = fopen(file_name, "rb")) == NULL)
    {
		return ERROR_FILE;
    }
    
    try 
    {
        fread(&bitmapfh.bfType, 1, sizeof(bitmapfh.bfType), image);
        fread(&bitmapfh.bfSize, 1, sizeof(bitmapfh.bfSize), image);
        fread(&bitmapfh.bfReserved1, 1, sizeof(bitmapfh.bfReserved1), image);
        fread(&bitmapfh.bfReserved2, 1, sizeof(bitmapfh.bfReserved2), image);
        fread(&bitmapfh.bfOffBits, 1, sizeof(bitmapfh.bfOffBits), image);
        
        if (fread(&bitmapih, 1, sizeof(bitmapih), image) != sizeof(bitmapih))
        {
            fclose(image);
            return ERROR_RW;
        }
    } 
    catch (exception)  // Не уверен, что будет работать. В c++ пока не просёк способ работы исключений
    {
        fclose(image);
        return ERROR_RW;
    }
    
    fileSize = bitmapfh.bfSize;
    offbyte = bitmapfh.bfOffBits;
	bitsp = bitmapih.biBitCount;
	rowLenght = 24 * bitmapih.biWidth * 4 / 32;
	if (rowLenght % 4 != 0)
		flag = true;
	if (bitsp == 24)
	{
		k = bitmapih.biSizeImage;
        
		PixelArray = new Pixel*[bitmapih.biHeight];
        
		for (int i = 0; i < bitmapih.biHeight; i++)
		{
			PixelArray[i] = new Pixel[bitmapih.biWidth];
		}
        
		fseek(image, offbyte, SEEK_SET);
        
		for (int i = 0; i < bitmapih.biHeight; i++)
		{
			for (int j = 0; j < bitmapih.biWidth; j++)
			{
				fread(&b, 1, 1, image);
				PixelArray[i][j].blue = b;
				fread(&g, 1, 1, image);
				PixelArray[i][j].green = g;
				fread(&r, 1, 1, image);
				PixelArray[i][j].red = r;
			}
			if (flag)
			{
				for (uint32_t l = 0; l < (4 - rowLenght % 4); l++)
					fread(&b, 1, 1, image);
			}
		}
    }
    else
    {
        fclose(image);
        return ERROR_FILE;
    }
	fclose(image);
    return SUCCESS;
}

STATUS IOBMP::setBmp(BMPImage image, char* name)
{
    FILE *resultImage;
    BitMapFH btfh;
    BitMapIH btih;
    uint8_t r, g, b;
    uint8_t ind = 0;
    bool flag = false;
    unsigned long rowLenght = 24 * image.getWidth() * 4 / 32;
    
    if (rowLenght % 4 != 0)
		flag = true;
    
	if ((resultImage = fopen(name, "wb")) == NULL)
	{
		return ERROR_FILE;
	}
    
    /* Ну типа когда мы считываем изображение - мы заносим в объект этого класса структуры bitmapfh/ih
     Поэтому нет смысла писать их вручную, а можно просто скопировать и изменить размер и то, что надо
     */
    
    btfh.bfType = 19778;
    btfh.bfSize = 0;
    btfh.bfReserved1 = 0;
    btfh.bfReserved2 = 0;
    btfh.bfOffBits = 54;
    
    btih.biSize = 0;
    btih.biPlanes = 1;
    btih.biBitCount = 24;
    btih.biCompression = 0;
    btih.biXPelsPerMeter = 2834;
    btih.biYPelsPerMeter = 2834;
    btih.biClrUsed = 0;
    btih.biClrImportant = 0;
    
    if (flag)
    {
        for (uint32_t l = 0; l < (4 - rowLenght % 4); l++)
            ind++;
        btfh.bfSize = image.getWidth() * image.getHeight() + ind * image.getHeight() + sizeof(btih) + sizeof(btfh);
        btih.biSizeImage = image.getWidth() * image.getHeight() + ind * image.getHeight();
        ind = 0;
    }
    else
    {
        btfh.bfSize = image.getWidth() * image.getHeight() + sizeof(btih) + sizeof(btfh);
        btih.biSizeImage = image.getWidth() * image.getHeight();
    }
    
    btih.biSize = sizeof(btih);
    btih.biWidth = image.getWidth();
    btih.biHeight = image.getHeight();
    
    fwrite(&btfh.bfType, 1, sizeof(btfh.bfType), resultImage);
    fwrite(&btfh.bfSize, 1, sizeof(btfh.bfSize), resultImage);
    fwrite(&btfh.bfReserved1, 1, sizeof(btfh.bfReserved1), resultImage);
    fwrite(&btfh.bfReserved2, 1, sizeof(btfh.bfReserved2), resultImage);
    fwrite(&btfh.bfOffBits, 1, sizeof(btfh.bfOffBits), resultImage);
    
    fwrite(&btih, 1, sizeof(btih), resultImage);

	Pixel tmp; // Для записи цветного файла

    for (int i = 0; i < image.getHeight(); i++)
    {
        for (int j = 0; j < image.getWidth(); j++)
        {
			tmp = image.getPixel(i, j);
			b = tmp.blue;
            fwrite(&b, 1, sizeof(uint8_t), resultImage);
			g = tmp.green;
            fwrite(&g, 1, sizeof(uint8_t), resultImage);
			r = tmp.red;
            fwrite(&r, 1, sizeof(uint8_t), resultImage);
        }
        if (flag)
        {
            for (uint32_t l = 0; l < (4 - rowLenght % 4); l++)
                fwrite("0", 1, sizeof(char), resultImage);
        }
    }
    fclose(resultImage);
    
    return SUCCESS;
}

void IOBMP::Release()
{
	for (int i = 0; i < bitmapih.biHeight; i++)
		delete [] PixelArray[i];
	delete [] PixelArray;
}

Pixel** IOBMP::getPixelArray()
{
    return PixelArray;
}

BitMapFH IOBMP::getBmpFileHeader()
{
    return bitmapfh;
}

BitMapIH IOBMP::getBmpInfoHeader()
{
    return bitmapih;
}