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
#include "Variance.h"

namespace SimLib
{
	void Variance::operator+=(double x)
	{
		this->sum0 += 1;
		this->sum1 += x;
		this->sum2 += x * x;
	}

	void Variance::operator-=(double x)
	{
		assert(this->sum0 > 0);

		this->sum0 -= 1;
		if(this->sum2 - x * x < 0.0)
		{
			this->sum1 = 0.0;
			this->sum2 = 0.0;
		}
		else
		{
			this->sum1 -= x;
			this->sum2 -= x * x;
		}
	}

	void Variance::Clear()
	{
		this->sum0 = 0;
		this->sum1 = 0.0;
		this->sum2 = 0.0;
	}

	double Variance::Biased()
	{
		return (this->sum0 > 1) ? (this->sum0*this->sum2 - this->sum1*this->sum1)/(this->sum0*this->sum0) : 0.0; 
	}

	double Variance::Unbiased()
	{
		return (this->sum0 > 1) ? (this->sum0*this->sum2 - this->sum1*this->sum1)/(this->sum0*(this->sum0-1)) : 0.0; 
	}

	double Variance::OfBiased(double* data, uint count)
	{
		double sum0 = 0;
		double sum1 = 0;
		double sum2 = 0;
		for(uint index = 0; index < count; index++)
		{
			sum0 += 1.0;
			sum1 += data[index];
			sum2 += data[index]*data[index];
		}
		return (sum0 != 0) ? (sum0*sum2 - sum1*sum1)/(sum0*sum0) : 0.0; 
	}

	double Variance::OfBiased(double* data, uint begin, uint end)
	{
		double sum0 = 0;
		double sum1 = 0;
		double sum2 = 0;
		for(uint index = begin; index < end; index++)
		{
			sum0 += 1.0;
			sum1 += data[index];
			sum2 += data[index]*data[index];
		}
		return (sum0 != 0) ? (sum0*sum2 - sum1*sum1)/(sum0*sum0) : 0.0; 
	}

	double Variance::OfBiased(std::vector<double>& data)
	{
		double sum0 = 0;
		double sum1 = 0;
		double sum2 = 0;
		for(std::vector<double>::iterator iter = data.begin(); iter != data.end(); iter++)
		{
			sum0 += 1.0;
			sum1 += (*iter);
			sum2 += (*iter)*(*iter);
		}
		return (sum0 != 0) ? (sum0*sum2 - sum1*sum1)/(sum0*sum0) : 0.0; 
	}

	double Variance::OfBiased(std::vector<double>& data, uint begin, uint end)
	{
		double sum0 = 0;
		double sum1 = 0;
		double sum2 = 0;
		for(uint index = begin; index < end; index++)
		{
			sum0 += 1.0;
			sum1 += data[index];
			sum2 += data[index]*data[index];
		}
		return (sum0 != 0) ? (sum0*sum2 - sum1*sum1)/(sum0*sum0) : 0.0; 
	}

	double Variance::OfBiasedNan(double* data, uint count)
	{
		double sum0 = 0;
		double sum1 = 0;
		double sum2 = 0;
		for(uint index = 0; index < count; index++)
		{
			if(data[index] == data[index])
			{
				sum0 += 1.0;
				sum1 += data[index];
				sum2 += data[index]*data[index];
			}
		}
		return (sum0 != 0) ? (sum0*sum2 - sum1*sum1)/(sum0*sum0) : 0.0; 
	}

	double Variance::OfBiasedNan(double* data, uint begin, uint end)
	{
		double sum0 = 0;
		double sum1 = 0;
		double sum2 = 0;
		for(uint index = begin; index < end; index++)
		{
			if(data[index] == data[index])
			{
				sum0 += 1.0;
				sum1 += data[index];
				sum2 += data[index]*data[index];
			}
		}
		return (sum0 != 0) ? (sum0*sum2 - sum1*sum1)/(sum0*sum0) : 0.0; 
	}

	double Variance::OfBiasedNan(std::vector<double>& data)
	{
		double sum0 = 0;
		double sum1 = 0;
		double sum2 = 0;
		for(std::vector<double>::iterator iter = data.begin(); iter != data.end(); iter++)
		{
			if((*iter) == (*iter))
			{
				sum0 += 1.0;
				sum1 += (*iter);
				sum2 += (*iter)*(*iter);
			}
		}
		return (sum0 != 0) ? (sum0*sum2 - sum1*sum1)/(sum0*sum0) : 0.0; 
	}

	double Variance::OfBiasedNan(std::vector<double>& data, uint begin, uint end)
	{
		double sum0 = 0;
		double sum1 = 0;
		double sum2 = 0;
		for(uint index = begin; index < end; index++)
		{
			if(data[index] == data[index])
			{
				sum0 += 1.0;
				sum1 += data[index];
				sum2 += data[index]*data[index];
			}
		}
		return (sum0 != 0) ? (sum0*sum2 - sum1*sum1)/(sum0*sum0) : 0.0; 
	}

	double Variance::OfUnbiased(double* data, uint count)
	{
		double sum0 = 0;
		double sum1 = 0;
		double sum2 = 0;
		for(uint index = 0; index < count; index++)
		{
			sum0 += 1.0;
			sum1 += data[index];
			sum2 += data[index]*data[index];
		}
		return (sum0 != 0) ? (sum0*sum2 - sum1*sum1)/(sum0*(sum0-1)) : 0.0; 
	}

	double Variance::OfUnbiased(double* data, uint begin, uint end)
	{
		double sum0 = 0;
		double sum1 = 0;
		double sum2 = 0;
		for(uint index = begin; index < end; index++)
		{
			sum0 += 1.0;
			sum1 += data[index];
			sum2 += data[index]*data[index];
		}
		return (sum0 != 0) ? (sum0*sum2 - sum1*sum1)/(sum0*(sum0-1)) : 0.0; 
	}

	double Variance::OfUnbiased(std::vector<double>& data)
	{
		double sum0 = 0;
		double sum1 = 0;
		double sum2 = 0;
		for(std::vector<double>::iterator iter = data.begin(); iter != data.end(); iter++)
		{
			sum0 += 1.0;
			sum1 += (*iter);
			sum2 += (*iter)*(*iter);
		}
		return (sum0 != 0) ? (sum0*sum2 - sum1*sum1)/(sum0*(sum0-1)) : 0.0; 
	}

	double Variance::OfUnbiased(std::vector<double>& data, uint begin, uint end)
	{
		double sum0 = 0;
		double sum1 = 0;
		double sum2 = 0;
		for(uint index = begin; index < end; index++)
		{
			sum0 += 1.0;
			sum1 += data[index];
			sum2 += data[index]*data[index];
		}
		return (sum0 != 0) ? (sum0*sum2 - sum1*sum1)/(sum0*(sum0-1)) : 0.0; 
	}

	double Variance::OfUnbiasedNan(double* data, uint count)
	{
		double sum0 = 0;
		double sum1 = 0;
		double sum2 = 0;
		for(uint index = 0; index < count; index++)
		{
			if(data[index] == data[index])
			{
				sum0 += 1.0;
				sum1 += data[index];
				sum2 += data[index]*data[index];
			}
		}
		return (sum0 != 0) ? (sum0*sum2 - sum1*sum1)/(sum0*(sum0-1)) : 0.0; 
	}

	double Variance::OfUnbiasedNan(double* data, uint begin, uint end)
	{
		double sum0 = 0;
		double sum1 = 0;
		double sum2 = 0;
		for(uint index = begin; index < end; index++)
		{
			if(data[index] == data[index])
			{
				sum0 += 1.0;
				sum1 += data[index];
				sum2 += data[index]*data[index];
			}
		}
		return (sum0 != 0) ? (sum0*sum2 - sum1*sum1)/(sum0*(sum0-1)) : 0.0; 
	}

	double Variance::OfUnbiasedNan(std::vector<double>& data)
	{
		double sum0 = 0;
		double sum1 = 0;
		double sum2 = 0;
		for(std::vector<double>::iterator iter = data.begin(); iter != data.end(); iter++)
		{
			if((*iter) == (*iter))
			{
				sum0 += 1.0;
				sum1 += (*iter);
				sum2 += (*iter)*(*iter);
			}
		}
		return (sum0 != 0) ? (sum0*sum2 - sum1*sum1)/(sum0*(sum0-1)) : 0.0; 
	}

	double Variance::OfUnbiasedNan(std::vector<double>& data, uint begin, uint end)
	{
		double sum0 = 0;
		double sum1 = 0;
		double sum2 = 0;
		for(uint index = begin; index < end; index++)
		{
			if(data[index] == data[index])
			{
				sum0 += 1.0;
				sum1 += data[index];
				sum2 += data[index]*data[index];
			}
		}
		return (sum0 != 0) ? (sum0*sum2 - sum1*sum1)/(sum0*(sum0-1)) : 0.0; 
	}
}
