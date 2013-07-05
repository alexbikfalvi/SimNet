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
#include "MlArrayDouble.h"
#include <SimLib/ExceptionUnsupported.h>
#include <SimLib/ExceptionMemory.h>
#include <SimLib/ExceptionArgument.h>

using namespace SimLib;

namespace MatLib
{
	MlArrayDouble::MlArrayDouble(MlSize ndim, const MlSize* dims, MlComplexity complexity, double* re, double* im)
		: MlArray(ndim, dims)
	{
#ifdef _MSC_VER
		// Create array
		this->mlArray = mxCreateNumericArray(ndim, dims, mxDOUBLE_CLASS, complexity);
		if(NULL == this->mlArray) throw ExceptionMemory(__FILE__, __LINE__, "Cannot allocate Matlab array of %u dimensions.", ndim);

		double* arrayRe = this->Re();
		double* arrayIm = this->Im();

		uint size = mxGetNumberOfElements(this->mlArray);

		// If data is not null, copy the data
		if(NULL != re)
		{
			for(uint index = 0; index < size; index++)
				arrayRe[index] = re[index];
		}

		// If matrix is complex
		if(mxCOMPLEX == complexity)
		{
			// If data is not null, copy the data
			if(NULL == im)
			{
				for(uint index = 0; index < size; index++)
					arrayIm[index] = im[index];
			}
		}

#elif __GNUC__
		// Set class ID
		this->classId = mxDOUBLE_CLASS;

		// Set the complexity
		this->complexity = complexity;
		
		// Allocate the real data
		this->re = alloc double[this->size];

		// If data is null, initialize with zero
		if(NULL == re)
		{
			for(uint index = 0; index < this->size; index++)
				this->re[index] = 0.0;
		}
		// Else, copy the data
		else
		{
			for(uint index = 0; index < this->size; index++)
				this->re[index] = re[index];
		}

		// If matrix is complex
		if(mxCOMPLEX == this->complexity)
		{
			// Allocate the imaginary data
			this->im = alloc double[this->size];

			// If data is null, initialize with zero
			if(NULL == im)
			{
				for(uint index = 0; index < this->size; index++)
					this->im[index] = 0.0;
			}
			// Else, copy the data
			else
			{
				for(uint index = 0; index < this->size; index++)
					this->im[index] = im[index];
			}
		}
		else this->im = NULL;
#endif
	}

	MlArrayDouble::MlArrayDouble(MlSize ndim, const MlSize* dims, std::vector<double>& re)
		: MlArray(ndim, dims)
	{
#ifdef _MSC_VER
		// Create array
		this->mlArray = mxCreateNumericArray(ndim, dims, mxDOUBLE_CLASS, mxREAL);
		if(NULL == this->mlArray) throw ExceptionMemory(__FILE__, __LINE__, "Cannot allocate Matlab array of %u dimensions.", ndim);

		double* arrayRe = this->Re();
		double* arrayIm = this->Im();

		uint size = mxGetNumberOfElements(this->mlArray);

		if(size != re.size()) throw ExceptionArgument(__FILE__, __LINE__, "Cannot allocate Matlab array: copy vector does not have the same number of elements (%u different from %u)", size, re.size());

		// If data is not null, copy the data
		for(uint index = 0; index < size; index++)
			arrayRe[index] = re[index];

#elif __GNUC__
		// Set class ID
		this->classId = mxDOUBLE_CLASS;

		// Set the complexity
		this->complexity = complexity;

		if(size != re.size()) throw ExceptionArgument(__FILE__, __LINE__, "Cannot allocate Matlab array: copy vector does not have the same number of elements (%u different from %u)", size, re.size());

		// Allocate the real data
		this->re = alloc double[this->size];

		// Copy the data
		for(uint index = 0; index < this->size; index++)
			this->re[index] = 0.0;

		this->im = NULL;
#endif		
	}

	MlArrayDouble::MlArrayDouble(const MlArrayDouble& obj)
	{
		// Throw an exception: cannot copy
		throw ExceptionUnsupported(__FILE__, __LINE__, "Cannot copy-construct an instance of MlArrayDouble class.");
	}

	MlArrayDouble::~MlArrayDouble()
	{
#ifdef _MSC_VER
		// Destroy array
		mxDestroyArray(this->mlArray);
#elif __GNUC__
		// Delete data
		delete[] this->re;
        delete[] this->im;
#endif
	}
}
