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
#include "Quartile.h"
#include "ExceptionArgument.h"

namespace SimLib
{
	Quartile::Quartile(std::vector<double>& data) : data(data)
	{
		// Sort the data
		std::sort(this->data.begin(), this->data.end());

		uint firstLo;
		uint firstHi;
		uint secondLo;
		uint secondHi;
		uint thirdLo;
		uint thirdHi;
		
		// Compute the second quartile
		this->second = this->Median(0, this->data.size()-1, secondLo, secondHi);
		// Compute the first quartile
		this->first = this->Median(0, secondLo, firstLo, firstHi);
		// Compute the third quartile
		this->third = this->Median(secondHi, this->data.size()-1, thirdLo, thirdHi);
		// Compute the minimum
		this->min = this->data[0];
		// Compute the maximum
		this->max = this->data[this->data.size()-1];
		// Compute the mean
		this->mean = 0.0;
		for(uint index = 0; index < this->data.size(); index++)
			this->mean += this->data[index];
		this->mean /= this->data.size();
	}

	Quartile::Quartile(std::vector<double>& data, uint begin, uint end) : data(data)
	{
		// Sort the data
		std::sort(&data[begin], &data[end]);

		uint firstLo;
		uint firstHi;
		uint secondLo;
		uint secondHi;
		uint thirdLo;
		uint thirdHi;
		
		// Compute the second quartile
		this->second = this->Median(begin, end-1, secondLo, secondHi);
		// Compute the first quartile
		this->first = this->Median(begin, secondLo, firstLo, firstHi);
		// Compute the third quartile
		this->third = this->Median(secondHi, end-1, thirdLo, thirdHi);
		// Compute the minimum
		this->min = this->data[begin];
		// Compute the maximum
		this->max = this->data[end-1];
		// Compute the mean
		this->mean = 0.0;
		for(uint index = begin; index < end; index++)
			this->mean += this->data[index];
		this->mean /= (end - begin);
	}

	Quartile::Quartile(double* data, uint count) : data(copy)
	{
		// Copy the data
		this->copy.resize(count, 0);
		memcpy(&this->data[0], data, count*sizeof(double));

		// Sort the data
		std::sort(this->data.begin(), this->data.end());

		uint firstLo;
		uint firstHi;
		uint secondLo;
		uint secondHi;
		uint thirdLo;
		uint thirdHi;
		
		// Compute the second quartile
		this->second = this->Median(0, this->data.size()-1, secondLo, secondHi);
		// Compute the first quartile
		this->first = this->Median(0, secondLo, firstLo, firstHi);
		// Compute the third quartile
		this->third = this->Median(secondHi, this->data.size()-1, thirdLo, thirdHi);
		// Compute the minimum
		this->min = this->data[0];
		// Compute the maximum
		this->max = this->data[this->data.size()-1];
		// Compute the mean
		this->mean = 0.0;
		for(uint index = 0; index < this->data.size(); index++)
			this->mean += this->data[index];
		this->mean /= this->data.size();
		// Compute the inter-quartile range
		this->iqr = this->third - this->first;
		// Compute the fence and outliers : < Q1 - 1.5*IQR / > Q3 + 1.5*IQR
		double boundLo = this->first - 1.5 * this->iqr;
		double boundHi = this->third + 1.5 * this->iqr;
		uint countOutLo = 0;
		uint countOutHi = 0;

		// Lower range
		{
			uint index = 0;
			for(; (index < this->data.size()) ? this->data[index] < boundLo : false; index++)
				countOutLo++;
			// Save lower fence
			this->fenceLo = this->data[index];
		}

		// Higher range
		{
			uint index = this->data.size();
			for(; (index > 0) ? this->data[index-1] > boundHi : false; index--)
				countOutHi++;
			// Save upper fence
			this->fenceHi = this->data[index];
		}

		// Save outliers
		this->outliers.resize(countOutLo + countOutHi);

		for(uint idxi = 0, idxo = 0; idxi < this->data.size(); idxi++)
		{
			if((this->data[idxi] < boundLo) || (this->data[idxi] > boundHi))
				this->outliers[idxo++] = this->data[idxi];
		}
	}

	double Quartile::Median(uint begin, uint end, uint& midLo, uint& midHi)
	{
		uint size = end - begin + 1;

		if(size == 0) throw ExceptionArgument(__FILE__, __LINE__, "Quartile::Median :size of data cannot be zero.");
	
		if(size % 2 == 0)
		{
			midLo = begin + size/2 - 1;
			midHi = begin + size/2;

			return (this->data[midLo] + this->data[midHi]) / 2.0;
		}
		else
		{
			midLo = begin + (size - 1)/2;
			midHi = midLo;

			return this->data[midLo];
		}
	}
}