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
#include "MlArray.h"
#include <SimLib/ExceptionUnsupported.h>

using namespace SimLib;

namespace MatLib
{
	MlArray::MlArray(MlSize ndim, const MlSize* dims)
	{
#if __GNUC__
		// Set the number of dimensions
		this->ndim = ndim;
		// Initialize the size
		this->size = (this->ndim > 0) ? 1 : 0;
		
		// Copy the dimensions and calculate the size
		this->dims = alloc MlSize[this->ndim];
		for(uint index = 0; index < this->ndim; index++)
		{
			this->dims[index] = dims[index];
			this->size *= this->dims[index];
		}
#endif
	}

	MlArray::MlArray(const MlArray& obj)
	{
		// Throw an exception: cannot copy
		throw ExceptionUnsupported(__FILE__, __LINE__, "Cannot copy-construct an instance of MlArray class.");
	}

	MlArray::~MlArray()
	{
#if __GNUC__
		// Delete dimensions
		delete[] this->dims;
#endif
	}
}
