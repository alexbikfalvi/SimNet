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
#include "MlArrayLogical.h"
#include <SimLib/ExceptionUnsupported.h>

using namespace SimLib;

namespace MatLib
{
	MlArrayLogical::MlArrayLogical(MlSize ndim, const MlSize* dims, bool* logicals)
		: MlArray(ndim, dims)
	{
#ifdef _MSC_VER
		// Create array
		this->mlArray = mxCreateLogicalArray(ndim, dims);

		bool* arrayLogicals = this->Logicals();

		uint size = mxGetNumberOfElements(this->mlArray);

		// If data is not null, copy the data
		if(NULL != logicals)
		{
			for(uint index = 0; index < size; index++)
				arrayLogicals[index] = logicals[index];
		}

#elif __GNUC__
		// Set class ID
		this->classId = mxLOGICAL_CLASS;
	
		// Allocate the logical data
		this->logicals = alloc bool[this->size];

		// If data is null, initialize with false
		if(NULL == logicals)
		{
			for(uint index = 0; index < this->size; index++)
				this->logicals[index] = false;
		}
		// Else, copy the data
		else
		{
			for(uint index = 0; index < this->size; index++)
				this->logicals[index] = logicals[index];
		}
#endif
	}

	MlArrayLogical::MlArrayLogical(const MlArrayLogical& obj)
	{
		// Throw an exception: cannot copy
		throw ExceptionUnsupported(__FILE__, __LINE__, "Cannot copy-construct an instance of MlArrayLogical class.");
	}

	MlArrayLogical::~MlArrayLogical()
	{
#ifdef _MSC_VER
		// Destroy array
		mxDestroyArray(this->mlArray);
#elif __GNUC__
		// Delete data
		delete[] this->logicals;
#endif
	}
}
