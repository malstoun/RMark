//
//  OCR.h
//  OCR
//
//  Created by Denis Hananein on 17.05.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef OCR_OCR_h
#define OCR_OCR_h

#include <iostream>
#include <math.h>
#include <vector>

#include "bmp.h"
#include "complex.h"



using namespace std;


class OCR {
    
    public:
    
    OCR(BITMAPFILEHEADER, BITMAPINFOHEADER, FILE*);
    ~OCR();
    
    
    private: 
    
    //структура вектора
    struct vectXY{
        //два конструктора для удобства
        vectXY();
        vectXY(int x, int y){
            this->x = (double)x;
            this->y = (double)y;
        }
        int x;
        int y;
    };
    
    //положение в пространстве (насколько символ сдвинут от краев)
    struct margin{
        int top;
        int left;
    };
    
    
    FILE *image;
    BITMAPFILEHEADER bitmapfh; 
    BITMAPINFOHEADER bitmapih;
    
    Pixel ** pixels;                    //пиксели изображения   
    vector<vectXY> curve;               //массив с координатами
//    Complex* fourier;                   //массив комплексных чисел для БПФ
    margin symbolMargin;
    
    unsigned long rowLength;            //временно, для наглядного вывода результата
    
    // methods
    
//    void resizeSymbol();                // масштабирование символов до определенного значения
    void symbolVector();                   // создание массива с координатами точек. как бы кривая буквы.
    int FFTOCR();                  // преобразование фурье
    
    void computerMargin();              //  определение отступов
    
    int getLuminion(int y, int x);      // получение яркости
    
    
    void result();          // временная выресовка картинки
    
};



#endif
