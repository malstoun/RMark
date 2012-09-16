//
//  IOBMP.h
//  Limin
//
//  Created by Андрей Рацеров on 29.03.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef Limin_IOBMP_h
#define Limin_IOBMP_h

#include "BMP.h"

enum STATUS 
{
    SUCCESS,
    ERROR_RW,
    ERROR_FILE,
    ERROR_UNTITLE
};

class IOBMP 
{
    BitMapIH bitmapih;
    BitMapFH bitmapfh;
    Pixel** PixelArray;
    
public:
    STATUS getBmp(char* file_name);
    STATUS setBmp(BMPImage image, char* name);
    Pixel** getPixelArray();
    BitMapFH getBmpFileHeader();
    BitMapIH getBmpInfoHeader();
    void Release();
};


#endif
