//
//  PImage.h
//  Limin
//
//  Created by ‚ on 27.01.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef Limin_Base_h
#define Limin_Base_h
#define _USE_MATH_DEFINES
#include <math.h>
#include "BMP.h"

int round(double a);
bool isWhite(Pixel p);
float rad(float a);
uint8_t getLight(Pixel px);
void setPixel(Pixel &px, uint8_t light);
int min3(int a, int b, int c);

#endif