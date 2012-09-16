//
//  OCR.cpp
//  OCR
//
//  Created by Denis Hananein on 17.05.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "OCR.h"
#include "complex.h"
#include "FFT.h"

using namespace std;


/*
 * Конструктор
 * 
 */

OCR::OCR(BITMAPFILEHEADER fHeader, BITMAPINFOHEADER iHeader, FILE* image){
    
    this->bitmapfh = fHeader;
    this->bitmapih = iHeader;
    this->image = image;
    
    this->rowLength = 24 * this->bitmapih.biWidth * 4 / 32;
    
    uint8_t r,g,b;
    fseek(this->image, this->bitmapfh.bfOffBits, SEEK_SET);
    
    //выделяем память под колонки
    this->pixels = new Pixel*[this->bitmapih.biHeight];
   
    //пробег по всем пикселям изображения
    for (int i = 0; i < this->bitmapih.biHeight; i++) {
        
        //выделяем память под строку. 
        this->pixels[i] = new Pixel[this->bitmapih.biWidth];
        
        for (int j = 0; j < this->bitmapih.biWidth; j++) {
  
            fgetpos(this->image, &this->pixels[i][j].pos);
            
            fread(&r, 1, 1, image);
            fread(&g, 1, 1, image);
            fread(&b, 1, 1, image);
            
            this->pixels[i][j].red = r;
            this->pixels[i][j].green = g;
            this->pixels[i][j].blue = b;
             
        }
        if (this->rowLength % 4 != 0){
            for (int l = 0; l < (4 - this->rowLength % 4); l++)
                fread(&b, 1, 1, image);
        }
    }
    
    this->symbolVector();
    this->result();
    
    this->FFTOCR();
    
}

/*
 * Деструктор
 *
 */

OCR::~OCR(){
    for (int i = 0; i < this->bitmapih.biHeight; i++) {
        delete [] this->pixels[i];
    }
    delete [] this->pixels;
}

/*
 * получение отступа символа от краёв
 *  (временно, пока не написан нормальный алгоритм выделения символов)
 */

void OCR::computerMargin(){
    
    //left
    int max = this->bitmapih.biWidth;

    for (int i = 0; i < this->bitmapih.biHeight; i++) {
        for (int j = 0; j < this->bitmapih.biWidth; j++) {
            if (getLuminion(i, j) < 400){
                if (max > j){
                    this->symbolMargin.left = j;
                    max = j;
                }
                break;
            }
        }
    }
    
    
    max = this->bitmapih.biHeight;
    for (int i = 0; i < this->bitmapih.biWidth; i++) {
        for (int j = 0; j < this->bitmapih.biHeight; j++) {
            if (getLuminion(j, i) < 400){
                if (max > j){
                    this->symbolMargin.top = j;
                    max = j;
                }
                break;
            }
        }
    }

    cout << this->symbolMargin.top << " + " << this->symbolMargin.left << endl;
    
}


/*
 * получение вектора из изображения 
 *
 */

void OCR::symbolVector(){
    
    //высчитываем отступы (если есть)
    this->computerMargin();
    
    
    //координаты
    int x, y;
    x = y = 0;
        
    //проход по всему изображнию
    int j;
    for (int i = 1; i < this->bitmapih.biHeight-1; i++) {
        
        j = 0;
        while (j < this->bitmapih.biWidth) {
            
            //если точка черная (или по крайней мере темная)
            if (getLuminion(i, j) < 500){

                //добавляем в вектор координаты
                this->curve.push_back(vectXY(j,i));
                
                //т.к. нужен только контур — пропускаем все ненужное
                while (j < this->bitmapih.biWidth && getLuminion(i, j) < 500 
                       && (getLuminion(i+1, j) < 500 && getLuminion(i-1, j) < 500)) {
                    j++;
                }
                
                //если не конец изображения — добавляем точку в массив.
                if (j != this->bitmapih.biWidth){ 
                    x = (getLuminion(i, j) < 500) ? j : j-1;
                    this->curve.push_back(vectXY(x,i));
                }
            }
            j++; 
        }
    }
    
//    //прогон по последней строчке
//    // !important Почему-то не ловит последнюю строчку.
//    // TODO: исправить
//    for (int i = 0; i < this->bitmapih.biWidth; i++){
//        if (getLuminion(this->bitmapih.biHeight-1, i) < 600){
//            this->curve.push_back(vectXY(i, this->bitmapih.biHeight-2));
//            
//        }
////        cout << getLuminion(this->bitmapih.biHeight, i) << endl;
//    }

    
//    cout << this->bitmapih.biHeight-1;
//    for (int i=0; i < this->bitmapih.biHeight; i++){
//        for (int j=0; j < this->bitmapih.biWidth; j++){
//            cout << getLuminion(i, j) << " |" << i << ";"<< j << " # ";
//        }
//        cout << endl;
//    }
}

/*
 *
 */


int OCR::FFTOCR(){

    Complex* fourier = new Complex[this->curve.size()];
    
    for (int i = 0; i < this->curve.size(); i++){

        fourier[i].RDate = this->curve[i].x;
        fourier[i].IDate = this->curve[i].y;
        
        cout << this->curve[i].x << " / " << this->curve[i].y << " # ";
    }

    FFT* FastFT = new FFT;
    FastFT->FFTRealization(fourier, this->curve.size(), true);
        
    cout << endl<< endl;
    
    int N = (this->curve.size() > 100) ? 100 : this->curve.size();
    
    double FFTranformationResult = 0;
    for (int i = 0; i < N; i++){
        FFTranformationResult+= fourier[i].RDate + fourier[i].IDate;
    }
    
    cout << FFTranformationResult;
    
    delete FastFT;
    delete [] fourier;
    
    return 0;
}


/*
 *
 */

int OCR::getLuminion(int y, int x){
    return this->pixels[y][x].red + this->pixels[y][x].green + this->pixels[y][x].blue;
}


void OCR::result(){
    
    FILE* resultImage = NULL;
    
    if ((resultImage = fopen("resultImage.bmp", "w")) == NULL){
		printf("Unable to open resfile");
	}
    
    fwrite(&this->bitmapfh.bfType, 1, sizeof(this->bitmapfh.bfType), resultImage);
    fwrite(&this->bitmapfh.bfSize, 1, sizeof(this->bitmapfh.bfSize), resultImage);
    fwrite(&this->bitmapfh.bfReserved1, 1, sizeof(this->bitmapfh.bfReserved1), resultImage);
    fwrite(&this->bitmapfh.bfReserved2, 1, sizeof(this->bitmapfh.bfReserved2), resultImage);
    fwrite(&this->bitmapfh.bfOffBits, 1, sizeof(this->bitmapfh.bfOffBits), resultImage);
    
    fwrite(&this->bitmapih, 1, sizeof(this->bitmapih), resultImage);

    uint8_t white = 255;
    
    for (int i = 0; i < this->bitmapih.biHeight; i++){
        for (int j = 0; j < this->bitmapih.biWidth; j++){
            
            fwrite(&white, 1, sizeof(char), resultImage);
            fwrite(&white, 1, sizeof(char), resultImage);
            fwrite(&white, 1, sizeof(char), resultImage);
        }
        if (this->rowLength % 4 !=0){
            for (int l = 0; l < (4 - this->rowLength % 4); l++)
                fwrite("0", 1, sizeof(char), resultImage);
    
        }
    }
    
    for (int i = 0; i < this->curve.size(); i++) {
       
        int x = this->curve[i].x - this->symbolMargin.left;
        int y = this->curve[i].y - this->symbolMargin.top;
        
        fsetpos(resultImage, &this->pixels[y][x].pos);
        
        fwrite("0", 1, sizeof(char), resultImage);
        fwrite("0", 1, sizeof(char), resultImage);
        fwrite("0", 1, sizeof(char), resultImage);
    }

    
    fclose(resultImage);
    
}

