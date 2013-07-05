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
#include "Sum.h"

namespace SimLib
{
	double Sum::Of(double* data, uint count)
	{
		double sum = 0.0;
		for(uint index = 0; index < count; index++)
			sum += data[index];
		return sum;
	}

	uint Sum::Of(std::vector<uint>& data)
	{
		uint sum = 0;
		for(std::vector<uint>::iterator iter = data.begin(); iter != data.end(); iter++)
			sum += *iter;
		return sum;
	}

	double Sum::Of(std::vector<double>& data)
	{
		double sum = 0.0;
		for(std::vector<double>::iterator iter = data.begin(); iter != data.end(); iter++)
			sum += *iter;
		return sum;
	}
}
