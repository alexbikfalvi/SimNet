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
#include "RegressionLeastSquares.h"

namespace SimLib
{
	RegressionLeastSquares::RegressionLeastSquares()
	{
		this->sumY2 = 0.0;
		this->sumXY = 0.0;
		this->sumX2 = 0.0;
		this->sumYYplus1 = 0.0;
		this->sumXYplus1 = 0.0;

		this->a = 0.0;
		this->b = 0.0;
	}

	void RegressionLeastSquares::Add(double x, double y, double yPlus1)
	{
		this->sumY2 += y * y;
		this->sumXY += x * y;
		this->sumX2 += x * x;
		this->sumYYplus1 += y * yPlus1;
		this->sumXYplus1 += x * yPlus1;

		this->den = this->sumY2*this->sumX2 - this->sumXY*this->sumXY;
		this->a = (this->den != 0.0) ? (this->sumX2*this->sumYYplus1 - this->sumXY*this->sumXYplus1) / this->den : 0.0;
		this->b = (this->den != 0.0) ? (this->sumY2*this->sumXYplus1 - this->sumXY*this->sumYYplus1) / this->den : 0.0;

		this->varYminusYhat += yPlus1 - this->a*y - this->b*x;
		this->varY += y;
	}

	void RegressionLeastSquares::Clear()
	{
		this->sumY2 = 0.0;
		this->sumXY = 0.0;
		this->sumX2 = 0.0;
		this->sumYYplus1 = 0.0;
		this->sumXYplus1 = 0.0;

		this->a = 0.0;
		this->b = 0.0;

		this->varYminusYhat.Clear();
		this->varY.Clear();
	}

	RegressionLeastSquares RegressionLeastSquares::Of(double* x, double* y, uint begin, uint end)
	{
		RegressionLeastSquares reg;
		for(uint index = begin; index < end-1; index++)
		{
			reg.Add(x[index], y[index], y[index+1]);
		}
		return reg;
	}

	RegressionLeastSquares RegressionLeastSquares::OfNan(double* x, double* y, uint begin, uint end)
	{
		RegressionLeastSquares reg;
		for(uint index = begin; index < end-1; index++)
		{
			if((x[index] == x[index]) && (y[index] == y[index]) && (y[index+1] == y[index+1]))
			{
				reg.Add(x[index], y[index], y[index+1]);
			}
		}
		return reg;
	}
}
