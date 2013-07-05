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
#include "FitEmpirical.h"
#include "ExceptionArgument.h"

namespace SimLib
{
	FitEmpirical::FitEmpirical(double* data, uint size)
	{
		for(uint index = 0; index < size; index++) this->Add(data[index]);
	}

	FitEmpirical::FitEmpirical(std::vector<double>& data)
	{
		for(std::vector<double>::iterator iter = data.begin(); iter != data.end(); iter++) this->Add(*iter);
	}

	void FitEmpirical::Clear()
	{
		this->data.clear();
		while(!this->samples.empty()) this->samples.pop();
	}

	void FitEmpirical::Add(double value)
	{
		// Add value to the empirical data set
		this->data.insert(value);
		this->samples.push(value);
		assert(this->data.size() == this->samples.size());
	}

	void FitEmpirical::Remove()
	{
		// Remove value from the empirical data set
		if(!this->samples.empty())
		{
			this->data.erase(this->data.find(this->samples.front()));
			this->samples.pop();
			assert(this->data.size() == this->samples.size());
		}
	}

	double FitEmpirical::Min()
	{
		return this->data.empty() ? std::numeric_limits<double>::quiet_NaN() : *this->data.begin();
	}

	double FitEmpirical::Max()
	{
		return this->data.empty() ? std::numeric_limits<double>::quiet_NaN() : *this->data.rbegin();
	}

	double FitEmpirical::Quantile(double prob)
	{
		// Calculate the quantile for the specified probability
		if((prob < 0) || (prob >= 1)) throw ExceptionArgument(__FILE__, __LINE__, "FitEmpirical::Quantile : the probability must be in the interval [0,1) (current value is %lf).", prob);

		// If the data set is empty, return NaN
		if(this->data.empty())
		{
			return std::numeric_limits<double>::quiet_NaN();
		}
		// Else, calculate the quantile
		else
		{
			double step = 1.0/this->data.size();
			uint index = 0;
			for(std::multiset<double>::iterator iter = this->data.begin(); iter != this->data.end(); iter++, index++)
				if((index*step <= prob) && ((index+1.0)*step > prob))
					return *iter;

			// Return the largest value
			return *this->data.rbegin();
		}
	}

	std::vector<double>& FitEmpirical::Data()
	{
		this->vdata.resize(this->data.size());

		uint index = 0;
		for(std::multiset<double>::iterator iter = this->data.begin(); iter != this->data.end(); iter++, index++)
			this->vdata[index] = *iter;

		return this->vdata;
	}

	std::vector<double>& FitEmpirical::Cdf()
	{
		this->cdf.resize(this->data.size());

		for(uint index = 0; index < this->cdf.size(); index++)
			this->cdf[index] = (double)(index + 1.0) / this->cdf.size();

		return this->cdf;
	}
}