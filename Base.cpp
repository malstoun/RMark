#include "Base.h"

uint8_t getLight(Pixel px)
{
	uint8_t light = px.red * 0.3 + px.green * 0.59 + px.blue * 0.11;

    return light;
}

int round(double a)
{
    if (a < 0) 
    {
        return (int)(a - .5);
    }
    else
    {
        return (int)(a + .5);
    }
}

bool isWhite(Pixel p)
{
    if (p.red == 255 && p.green == 255 && p.blue == 255) 
    {
        return true;
    }
    else
        return false;
}

float rad(float a)
{
    return (a / 180. * M_PI);
}

void setPixel(Pixel &px, uint8_t light)
{
    px.red = light;
    px.green = light;
    px.blue = light;
}

int min3(int a, int b, int c)
{
	return (a > b ? (b > c ? c : b) : (a > c ? c : a));
}