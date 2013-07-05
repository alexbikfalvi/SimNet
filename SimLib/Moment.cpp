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
#include "Moment.h"
#include "ExceptionUnsupported.h"

namespace SimLib
{
	void Moment::operator +=(double value)
	{
		double value2 = value*value;
		double value3 = value*value2;
		double value4 = value*value3;

		this->sum0 += 1;
		this->sum1 += value;
		this->sum2 += value2;
		this->sum3 += value3;
		this->sum4 += value4;

		double sum0_2 = sum0*sum0;
		double sum0_3 = sum0*sum0_2;
		double sum0_4 = sum0*sum0_3;
		double sum1_2 = sum1*sum1;
		double sum1_3 = sum1*sum1_2;
		double sum1_4 = sum1*sum1_3;

		this->mean = (sum0 > 0) ? sum1 / sum0 : 0.0;
		this->varU = (sum0 > 1) ? (sum0*sum2 - sum1_2) / (sum0_2 - sum0) : 0.0;
		this->varB = (sum0 > 1) ? (sum0*sum2 - sum1_2) / sum0_2 : 0.0;
		if(this->varB < 0)
		{
			if(std::fabs(this->varB) < std::numeric_limits<double>::epsilon()) this->varB = 0.0;
			else throw ExceptionUnsupported(__FILE__, __LINE__, "Moment::operator+ : biased variance less than zero (%lf)", this->varB);
		}
		this->std = sqrt(varB);
		this->skew = (sum0 > 0) && (std > 0) ? (sum0_2*sum3 - 3*sum0*sum1*sum2 + 2*sum1_3) / (sum0_3*std*std*std) : 0.0;
		this->kurt = (sum0 > 0) && (std > 0) ? ((sum0_3*sum4 - 4*sum0_2*sum1*sum3 + 6*sum0*sum1_2*sum2 - 3*sum1_4) / (sum0_4*varB*varB)) : 0.0;
	}

	void Moment::operator -=(double value)
	{
		if(0 == this->sum0) throw ExceptionUnsupported(__FILE__, __LINE__, "Moment::operator- : subtract operation cannot be performed (number of elements is zero).");

		double value2 = value*value;
		double value3 = value*value2;
		double value4 = value*value3;
        
		// Decrease the number of elements
		this->sum0 -= 1;

		// If the second/fourth sum is close to zero
		if((this->sum2 - value2 < 0.0) || (this->sum4 - value4 < 0.0) || (this->sum0 == 0))
		{
			this->sum1 = 0.0;
			this->sum2 = 0.0;
			this->sum3 = 0.0;
			this->sum4 = 0.0;
		}
		else
		{
			this->sum1 -= value;
			this->sum2 -= value2;
			this->sum3 -= value3;
			this->sum4 -= value4;
		}


		double sum0_2 = sum0*sum0;
		double sum0_3 = sum0*sum0_2;
		double sum0_4 = sum0*sum0_3;
		double sum1_2 = sum1*sum1;
		double sum1_3 = sum1*sum1_2;
		double sum1_4 = sum1*sum1_3;

		this->mean = (sum0 > 0) ? sum1 / sum0 : 0.0;
		this->varU = (sum0 > 1) ? (sum0*sum2 - sum1_2) / (sum0_2 - sum0) : 0.0;
		this->varB = (sum0 > 1) ? (sum0*sum2 - sum1_2) / sum0_2 : 0.0;

		if(this->varB < 0)
		{
			if(std::fabs(this->varB) < std::numeric_limits<double>::epsilon()) this->varB = 0.0;
			else throw ExceptionUnsupported(__FILE__, __LINE__, "Moment::operator- : biased variance less than zero (%lf)", this->varB);
		}
		this->std = sqrt(varB);
		this->skew = (sum0 > 0) && (std > 0) ? (sum0_2*sum3 - 3*sum0*sum1*sum2 + 2*sum1_3) / (sum0_3*std*std*std) : 0.0;
		this->kurt = (sum0 > 0) && (std > 0) ? ((sum0_3*sum4 - 4*sum0_2*sum1*sum3 + 6*sum0*sum1_2*sum2 - 3*sum1_4) / (sum0_4*varB*varB)) : 0.0;
	}

	void Moment::Add(double value)
	{
		(*this) += value;
		
		this->samples.push(value);
		assert(this->samples.size() == this->sum0);
	}

	void Moment::Remove()
	{
		if(!this->samples.empty())
		{
			(*this) -= this->samples.front();

			this->samples.pop();
			assert(this->samples.size() == this->sum0);
		}
	}

	void Moment::Clear()
	{
		this->sum0 = 0;
		this->sum1 = 0.0;
		this->sum2 = 0.0;
		this->sum3 = 0.0;
		this->sum4 = 0.0;
		while(!this->samples.empty()) this->samples.pop();
	}
}
