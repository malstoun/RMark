//
//  complex.h
//  complex
//
//  Created by Nikita Mostovoy on 12.06.12.

#ifndef ComplexNumb
#define ComplexNumb

class Complex
{
public:
    long double RDate,
    IDate;
    
    //Arithmetic methods
    Complex operator= (const Complex&);
    Complex operator+ (Complex&);
    Complex operator- (Complex&);
    Complex operator* (Complex&);
    void	operator+=(const Complex&);
    void    operator*=(const Complex& A);
    void    operator-=(const Complex&);
    void    operator/=(long double D);
    
    //Constructors
    Complex(long double R, long double I);
//    Complex(int R, int I);
    Complex();
    Complex(const Complex&);
    
    
    
};



#endif