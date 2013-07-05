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
#include "FilterSmooth.h"
#include "ExceptionArgument.h"

namespace SimLib
{
	FilterSmooth::FilterSmooth(int left, int right, std::vector<double>& coeff)
	{
		if(left + right + 1 != coeff.size()) throw ExceptionArgument(__FILE__, __LINE__, "The size of the coefficients vector must be left + right + 1.");

		this->left = left;
		this->right = right;
		this->coeff = coeff;
	}

	FilterSmooth::~FilterSmooth()
	{
	}
			
	double FilterSmooth::operator()(double* input, double* output, uint index, uint size)
	{
		// Find the begin and end index for the current index and input size
		int indexBegin = index - this->left;
		int indexEnd = index + this->right;

		// Calculate the begin and end delta
		int deltaBegin = (indexBegin < 0) ? -indexBegin : 0;
		int deltaEnd = (indexEnd > (int)size) ? indexEnd - size : 0;

		double result = 0;
		for(int idx = deltaBegin; idx < (int)this->coeff.size() - deltaEnd; idx++)
		{
			assert(idx >= 0);
			assert(indexBegin + idx >= 0);
			assert(idx < (int)this->coeff.size());
			assert(indexBegin + idx < (int)size);

			result += input[indexBegin + idx] * this->coeff[idx];
		}
		return result;
	}

	double FilterSmooth::operator()(std::vector<double>& input, std::vector<double>& output, uint index)
	{
		// Find the begin and end index for the current index and input size
		int indexBegin = index - this->left;
		int indexEnd = index + this->right;

		// Calculate the begin and end delta
		int deltaBegin = (indexBegin < 0) ? -indexBegin : 0;
		int deltaEnd = (indexEnd > (int)input.size()) ? indexEnd - input.size() : 0;

		double result = 0;
		for(int idx = deltaBegin; idx < (int)this->coeff.size() - deltaEnd; idx++)
		{
			assert(idx >= 0);
			assert(indexBegin + idx >= 0);
			assert(idx < (int)this->coeff.size());
			assert(indexBegin + idx < (int)input.size());

			result += input[indexBegin + idx] * this->coeff[idx];
		}
		return result;
	}
}
