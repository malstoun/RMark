//
//  complex.cpp
//  complex
//
//  Created by Nikita Mostovoy on 12.06.12.

#include "complex.h"

Complex::Complex()
{
	IDate = RDate = 0;
}

Complex::Complex(long double R, long double I)
{
	RDate = R; 
	IDate = I;
}

//Complex::Complex(int R, int I)
//{
//	RDate = R; 
//	IDate = I;
//}

Complex::Complex(const Complex& A)
{
	RDate = A.RDate;
	IDate = A.IDate;
}

Complex Complex::operator=(const Complex& A)
{
	RDate = A.RDate;
	IDate = A.IDate;
	return A;
}

Complex Complex::operator+(Complex& A)
{
	return(Complex(RDate+A.RDate,IDate+A.IDate));
}

Complex Complex::operator-(Complex& A)
{
	return(Complex(RDate-A.RDate,IDate-A.IDate));
}

Complex Complex::operator*(Complex& A)
{
	return(Complex(RDate*A.RDate-IDate*A.IDate,RDate*A.IDate+IDate*A.RDate));
}

void Complex::operator+=(const Complex& A)
{
	RDate += A.RDate, IDate += A.IDate;
}

void Complex::operator-=(const Complex& A)
{
	RDate -= A.RDate, IDate -= A.IDate;
}

void Complex::operator*=(const Complex& A)
{
	
    long double RDate_2 = RDate;
    RDate_2 = RDate*A.RDate-IDate*A.IDate;
    IDate = RDate*A.IDate+IDate*A.RDate;
    RDate = RDate_2;
	
}

void Complex::operator/=(long double D)
{
	RDate/=D;
	IDate/=D;	
}

