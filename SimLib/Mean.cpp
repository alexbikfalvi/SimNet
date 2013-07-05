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
#include "Mean.h"

namespace SimLib
{
	void Mean::operator+=(double x)
	{
		this->sum0 += 1;
		this->sum1 += x;
	}

	void Mean::operator-=(double x)
	{
		if(this->sum0 <= 0) return;
		this->sum0 -= 1;
		this->sum1 -= x;
	}

	double Mean::operator()()
	{
		return (this->sum0 != 0) ? this->sum1/this->sum0 : 0.0; 
	}

	void Mean::Clear()
	{
		this->sum0 = 0;
		this->sum1 = 0.0;
	}

	double Mean::Of(double* data, uint count)
	{
		double sum0 = 0;
		double sum1 = 0;
		for(uint index = 0; index < count; index++)
		{
			sum0 += 1.0;
			sum1 += data[index];
		}
		return (sum0 != 0) ? sum1/sum0 : 0.0; 
	}

	double Mean::Of(double* data, uint begin, uint end)
	{
		double sum0 = 0;
		double sum1 = 0;
		for(uint index = begin; index < end; index++)
		{
			sum0 += 1.0;
			sum1 += data[index];
		}
		return (sum0 != 0) ? sum1/sum0 : 0.0; 
	}

	double Mean::Of(std::vector<double>& data)
	{
		double sum0 = 0;
		double sum1 = 0;
		for(std::vector<double>::iterator iter = data.begin(); iter != data.end(); iter++)
		{
			sum0 += 1.0;
			sum1 += (*iter);
		}
		return (sum0 != 0) ? sum1/sum0 : 0.0; 
	}

	double Mean::Of(std::vector<double>& data, uint begin, uint end)
	{
		double sum0 = 0;
		double sum1 = 0;
		for(uint index = begin; index < end; index++)
		{
			sum0 += 1.0;
			sum1 += data[index];
		}
		return (sum0 != 0) ? sum1/sum0 : 0.0; 
	}


	double Mean::OfNan(double* data, uint count)
	{
		double sum0 = 0;
		double sum1 = 0;
		for(uint index = 0; index < count; index++)
		{
			if(data[index] == data[index])
			{
				sum0 += 1.0;
				sum1 += data[index];
			}
		}
		return (sum0 != 0) ? sum1/sum0 : 0.0; 
	}

	double Mean::OfNan(double* data, uint begin, uint end)
	{
		double sum0 = 0;
		double sum1 = 0;
		for(uint index = begin; index < end; index++)
		{
			if(data[index] == data[index])
			{
				sum0 += 1.0;
				sum1 += data[index];
			}
		}
		return (sum0 != 0) ? sum1/sum0 : 0.0; 
	}

	double Mean::OfNan(std::vector<double>& data)
	{
		double sum0 = 0;
		double sum1 = 0;
		for(std::vector<double>::iterator iter = data.begin(); iter != data.end(); iter++)
		{
			if((*iter) == (*iter))
			{
				sum0 += 1.0;
				sum1 += (*iter);
			}
		}
		return (sum0 != 0) ? sum1/sum0 : 0.0; 
	}

	double Mean::OfNan(std::vector<double>& data, uint begin, uint end)
	{
		double sum0 = 0;
		double sum1 = 0;
		for(uint index = begin; index < end; index++)
		{
			if(data[index] == data[index])
			{
				sum0 += 1.0;
				sum1 += data[index];
			}
		}
		return (sum0 != 0) ? sum1/sum0 : 0.0; 
	}
}
