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

#pragma once

#include "Variance.h"

namespace SimLib
{
	class RegressionLeastSquares
	{
	private:
		double				sumY2;
		double				sumXY;
		double				sumX2;
		double				sumYYplus1;
		double				sumXYplus1;
		
		double				a;
		double				b;
		double				den;

		Variance			varYminusYhat;
		Variance			varY;

	public:
		RegressionLeastSquares();
		virtual ~RegressionLeastSquares() { }

		void							Add(double x, double y, double yPlus1);
		
		void							Clear();
				
		inline double					A() { return this->a; }
		inline double					B() { return this->b; }
		inline double					R2() { return (this->varY() != 0) ? 1.0 - this->varYminusYhat() / this->varY() : 1.0; }

		static RegressionLeastSquares	Of(double* x, double* y, uint begin, uint end);
		static RegressionLeastSquares	OfNan(double* x, double* y, uint begin, uint end);
	};
}
