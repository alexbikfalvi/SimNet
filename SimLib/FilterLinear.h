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

#include "Filter.h"

namespace SimLib
{
	/*
	 * Linear filter : solves the difference equation
	 *
	 * a[0]y[i] + a[1]y[i-1] + ... + a[n]y[i-n] = b[0]x[i] +  b[1]y[x-1] + ... + b[m]x[i-m]
	 *
	 * y[i] = (b[0]x[i] +  b[1]y[x-1] + ... + b[m]x[i-m] - a[1]y[i-1] - ... - a[n]y[i-n]) / a[0];
	 */

	class FilterLinear : public Filter
	{
	private:
		std::vector<double>	num;
		std::vector<double>	den;

	public:
		FilterLinear(std::vector<double>& num, std::vector<double>& den);
		FilterLinear(double a, double b);
		virtual ~FilterLinear();
		
		virtual double	operator()(double* input, double* output, uint index, uint size = 0);
		virtual double	operator()(std::vector<double>& input, std::vector<double>& output, uint index);
	};
}
