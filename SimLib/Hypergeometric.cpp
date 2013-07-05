/* 
 * Copyright (C) 2011 Alex Bikfalvi
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "Headers.h"
#include "Hypergeometric.h"
#include <boost/math/special_functions/gamma.hpp>

#define PRECISION   1E-16
#define ROUND(x)    ((x >= 0)?floor(x+0.5):ceil(x-0.5))

#ifndef M_PI
#define M_PI		3.14159265358979323846264338328      /* pi */
#endif

using namespace boost::math;

namespace SimLib
{
	double Hypergeometric2F1Series(double a, double b, double c, double z)
	{
		unsigned int i = 1;
		double term = a*b*z/c;
		double sum = 1+term;

		do
		{
			term *= ((a+i)*(b+i)*z)/((c+i)*(i+1));
			sum += term;
			i++;
		}
		while(fabs(term) > PRECISION);
		return sum;
	}

	Hypergeometric2F1::Hypergeometric2F1(double a, double b, double c)
	{
		this->a = a;
		this->b = b;
		this->c = c;

		this->c1 = fabs(c-a-b);
		this->c2 = tgamma<double>(c) * tgamma<double>(c-a-b) / (tgamma<double>(c-a) * tgamma<double>(c-b));
		this->c3 = tgamma<double>(c) * tgamma<double>(b-a) / (tgamma<double>(b) * tgamma<double>(c-a));
		this->c4 = tgamma<double>(c) * tgamma<double>(a-b) / (tgamma<double>(a) * tgamma<double>(c-b));
		this->c5 = tgamma<double>(c) * tgamma<double>(c-a-b) / (tgamma<double>(c-a) * tgamma<double>(c-b));
		this->c6 = tgamma<double>(c) * tgamma<double>(a+b-c) / (tgamma<double>(a) * tgamma<double>(b));
		this->c7 = cos(M_PI*(c-a-b));
		this->c8 = sin(M_PI*(c-a-b));
		this->c9 = cos(M_PI*a);
		this->c10 = cos(M_PI*b);
		this->c11 = sin(M_PI*a);
		this->c12 = sin(M_PI*b);
	}

	double Hypergeometric2F1::Real(double z)
	{
		if(fabs(z) < PRECISION) return 1;           // z = 0;
		else if(fabs(z-1) < PRECISION)              // z = 1;
		{
			if(this->c1 < PRECISION) return 1E300;
			else return this->c2;
		}

		if(z < -1)
		{
			double w = 1/(1-z);
			return pow(w,a) * Hypergeometric2F1Series(a,c-b,a-b+1,w) * this->c3 +
				   pow(w,b) * Hypergeometric2F1Series(b,c-a,b-a+1,w) * this->c4;
		}
		else if(z < 0)
		{
			double w = z/(z-1);
			return pow(1-w,a) * Hypergeometric2F1Series(a,c-b,c,w);
		}
		else if(z <= 0.5)
		{
			return Hypergeometric2F1Series(a,b,c,z);
		}
		else if(z <= 1)
		{
			double w = 1-z;
			return Hypergeometric2F1Series(a,b,a+b-c+1,w) * this->c5 +
				   pow(w,c-a-b) * Hypergeometric2F1Series(c-a,c-b,c-a-b+1,w) * this->c6;
		}
		else if(z <= 2)
		{
			double w = 1 - (1/z);
			return pow(1-w,a) * Hypergeometric2F1Series(a,a-c+1,a+b-c+1,w) * this->c5 +
				pow(1-w,a) * pow(w,c-a-b) * this->c7 * Hypergeometric2F1Series(1-b,c-b,c-a-b+1,w) * this->c6;
		}
		else
		{
			double w = 1/z;

			double h1 = pow(w,a) * Hypergeometric2F1Series(a,a-c+1,a-b+1,w) * this->c3;
			double h2 = pow(w,b) * Hypergeometric2F1Series(b-c+1,b,b-a+1,w) * this->c4;

			return this->c9 * h1 + this->c10 * h2;
		}
	}

	std::complex<double> Hypergeometric2F1::Complex(double z)
	{
		if(fabs(z) < PRECISION) return 1;           // z = 0;
		else if(fabs(z-1) < PRECISION)              // z = 1;
		{
			if(this->c1 < PRECISION) return 1E300;
			else return this->c2;
		}

		if(z < -1)
		{
			double w = 1/(1-z);
			return pow(w,a) * Hypergeometric2F1Series(a,c-b,a-b+1,w) * this->c3 +
				   pow(w,b) * Hypergeometric2F1Series(b,c-a,b-a+1,w) * this->c4;
		}
		else if(z < 0)
		{
			double w = z/(z-1);
			return pow(1-w,a) * Hypergeometric2F1Series(a,c-b,c,w);
		}
		else if(z <= 0.5)
		{
			return Hypergeometric2F1Series(a,b,c,z);
		}
		else if(z <= 1)
		{
			double w = 1-z;
			return Hypergeometric2F1Series(a,b,a+b-c+1,w) * this->c5 +
				   pow(w,c-a-b) * Hypergeometric2F1Series(c-a,c-b,c-a-b+1,w) * this->c6;
		}
		else if(z <= 2)
		{
			double w = 1 - (1/z);
			double re = pow(1-w,a) * Hypergeometric2F1Series(a,a-c+1,a+b-c+1,w) * this->c5 +
						pow(1-w,a) * pow(w,c-a-b) * this->c7 * Hypergeometric2F1Series(1-b,c-b,c-a-b+1,w) * this->c6;
			double im = -pow(1-w,a) * pow(w,c-a-b) * this->c8 * Hypergeometric2F1Series(c-a,1-a,c-a-b+1,w) * this->c6;
			return std::complex<double>(re, im);
		}
		else
		{
			double w = 1/z;

			double h1 = pow(w,a) * Hypergeometric2F1Series(a,a-c+1,a-b+1,w) * this->c3;
			double h2 = pow(w,b) * Hypergeometric2F1Series(b-c+1,b,b-a+1,w) * this->c4;

			double re = this->c9 * h1 + this->c10 * h2;
			double im = this->c11 * h1 + this->c12 * h2;
        
			return std::complex<double>(re, im);
		}
	}
}