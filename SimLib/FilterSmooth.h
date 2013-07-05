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
	class FilterSmooth : public Filter
	{
	private:
		int					left;
		int					right;
		std::vector<double>	coeff;

	public:
		FilterSmooth(int left, int right, std::vector<double>& coeff);
		virtual ~FilterSmooth();
		
		virtual double	operator()(double* input, double* output, uint index, uint size = 0);
		virtual double	operator()(std::vector<double>& input, std::vector<double>& output, uint index);
	};
}
