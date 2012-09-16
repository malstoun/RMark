//
//  FFT.h
//  FFT
//
//  Created by Nikita Mostovoy on 12.06.12.
//
// Tested with http://www.random-science-tools.com/maths/FFT.htm FFT Window function below: none

#ifndef FFTAlg
#define FFTAlg

#include "complex.h"

class FFT
{
public:
	//FFT algoritm
	//X - input array -> output array
	//N - length of X
	//Direction - flag of reverse FFT
	static void FFTRealization(Complex *X, int N, bool Direction);
	//use - FFT::FFTRealization(Complex*,int,bool)
    
    private:
    
    static unsigned char reserse256[256];
    
};

#endif