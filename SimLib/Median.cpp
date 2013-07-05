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
#include "Median.h"
#include "ExceptionArgument.h"

namespace SimLib
{

	double Median::Of(double* data, uint count)
	{
		std::vector<double> vec(data, data+count);
		return Median::Of(vec);
	}

	double Median::Of(double* data, uint begin, uint end)
	{
		std::vector<double> vec(data+begin, data+end);
		return Median::Of(vec);
	}

	double Median::Of(std::vector<double>& data)
	{
		if(data.size() == 0) throw ExceptionArgument(__FILE__, __LINE__, "Median::Of(data, begin, end) : begin cannot be equal to end.");
		std::sort(data.begin(), data.end());
		if(data.size() % 2 == 0)
			// Vector size is even : return mean between middle elements
			return (data[data.size()/2-1] + data[data.size()/2]) / 2.0;
		else
			// Vector size if odd : return middle element
			return data[(data.size()-1)/2];
	}

	double Median::Of(std::vector<double>& data, uint begin, uint end)
	{
		if(begin == end) throw ExceptionArgument(__FILE__, __LINE__, "Median::Of(data, begin, end) : begin cannot be equal to end.");
		std::sort(&data[begin], &data[end]);
		if(data.size() % 2 == 0)
			// Vector size is even : return mean between middle elements
			return (data[data.size()/2-1] + data[data.size()/2]) / 2.0;
		else
			// Vector size if odd : return middle element
			return data[(data.size()-1)/2];
	}
}