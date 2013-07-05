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
#include "Roots.h"
#include "ExceptionArgument.h"

namespace SimLib
{
	void Roots::Linear(double a, double b, std::complex<double>& r)
	{
		if(0 == a) throw ExceptionArgument(__FILE__, __LINE__, "Roots::Linear : parameter a cannot be zero.");

		if(0 == b) r = 0.0;
		else r = -b/a;
	}

	void Roots::Quadratic(double a, double b, double c, std::complex<double>& r1, std::complex<double>& r2)
	{
		if(0 == a) throw ExceptionArgument(__FILE__, __LINE__, "Roots::Quadratic : parameter a cannot be zero.");

		if(0 == c)
		{
			// If c is zero, one root is zero, and solve the linear equation
			r1 = 0.0;
			Roots::Linear(a, b, r2);
		}
		else
		{
			// Else
			std::complex<double> delta = b*b-4.0*a*c;
			r1 = (-b + std::sqrt(delta))/(2.0*a);
			r2 = (-b - std::sqrt(delta))/(2.0*a);
		}
	}

	void Roots::Cubic(double a, double b, double c, double d, std::complex<double>& r1, std::complex<double>& r2, std::complex<double>& r3)
	{
		if(0 == a) throw ExceptionArgument(__FILE__, __LINE__, "Roots::Cubic : parameter a cannot be zero.");

		if(0 == c)
		{
			// If c is zero, one root is zero, and solve the quadratic equation
			r1 = 0.0;
			Roots::Quadratic(a, b, c, r2, r3);
		}
		else
		{
			// Else
			std::complex<double> Q = std::sqrt(std::complex<double>(std::pow(2.0*b*b*b - 9.0*a*b*c + 27.0*a*a*d,2.0) - 4.0*std::pow(b*b-3.0*a*c,3.0)));
			std::complex<double> C = std::pow((Q + 2.0*b*b*b - 9.0*a*b*c + 27.0*a*a*d)/2.0, 0.333333333333);

			std::complex<double> c1(1.0, sqrt(3.0));
			std::complex<double> c2(1.0, -sqrt(3.0));

			r1 = -(b + C + (b*b-3.0*a*c)/C)/(3.0*a);
			r2 = -(b - C*c1/2.0 - c2*(b*b-3.0*a*c)/(2.0*C))/(3.0*a);
			r3 = -(b - C*c2/2.0 - c1*(b*b-3.0*a*c)/(2.0*C))/(3.0*a);
		}
	}

}
