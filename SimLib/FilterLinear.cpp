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
#include "FilterLinear.h"

namespace SimLib
{
	FilterLinear::FilterLinear(std::vector<double>& num, std::vector<double>& den)
	{
		this->num = num;
		this->den = den;
	}

	FilterLinear::FilterLinear(double a, double b)
	{
		this->num.resize(1);
		this->den.resize(2);

		this->num[0] = b;
		this->den[0] = 1.0;
		this->den[1] = a;
	}

	FilterLinear::~FilterLinear()
	{
	}

	double FilterLinear::operator()(double* input, double* output, uint index, uint size)
	{
		double result = 0.0;
		for(uint idx = 0; (idx < this->num.size()) && (index >= idx); idx++)
			result += input[index-idx] * this->num[idx];
		for(uint idx = 1; (idx < this->den.size()) && (index >= idx); idx++)
			result -= output[index-idx] * this->den[idx];
		return result / this->den[0];
	}

	double FilterLinear::operator()(std::vector<double>& input, std::vector<double>& output, uint index)
	{
		double result = 0.0;
		for(uint idx = 0; (idx < this->num.size()) && (index >= idx); idx++)
			result += input[index-idx] * this->num[idx];
		for(uint idx = 1; (idx < this->den.size()) && (index >= idx); idx++)
			result -= output[index-idx] * this->den[idx];
		return result / this->den[0];
	}
}
