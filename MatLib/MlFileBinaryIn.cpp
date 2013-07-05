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
#include "MlFileBinaryIn.h"
#include <SimLib/ExceptionIo.h>
#include <SimLib/ExceptionArgument.h>
#include <SimLib/ExceptionUnsupported.h>

using namespace SimLib;

namespace MatLib
{
	MlFileBinaryIn::MlFileBinaryIn(const char* fileName)
	{
		this->file.open(fileName, std::ios::binary | std::ios::in);
		if(!this->file.good()) throw ExceptionIo(__FILE__, __LINE__, "Opening the binary file failed.");
	}

	MlFileBinaryIn::~MlFileBinaryIn()
	{
		this->file.close();
	}

	MlVar MlFileBinaryIn::Read()
	{
		// Write the array to the file

		// Read the name size
		uint nameSize;
		this->file.read((char*)&nameSize, sizeof(uint));

		// Read the name
		char* name = alloc char[nameSize];
		this->file.read(name, nameSize * sizeof(char));

		// Read the class ID
		uint classId;
		this->file.read((char*)&classId, sizeof(uint));

		MlArray* mlArray;
		switch(classId)
		{
		case mxDOUBLE_CLASS: mlArray = this->ReadDouble(); break;
		case mxLOGICAL_CLASS: mlArray = this->ReadLogical(); break;
		default: throw ExceptionUnsupported(__FILE__, __LINE__, "MlFileBinaryIn::Read : array class ID %u is not supported.", classId);
		}


		// Create the variable
		MlVar var(name, mlArray);

		delete[] name;

		return var;
	}

	MlArray* MlFileBinaryIn::ReadDouble()
	{
		// Read the array dimensions
		uint ndim;
		this->file.read((char*)&ndim, sizeof(uint));

		// Read the dimensions
		uint* dims = alloc uint[ndim];
		for(uint index = 0; index < ndim; index++)
		{
			this->file.read((char*)&dims[index], sizeof(uint));
		}
		
		// Read the array size
		uint size;
		this->file.read((char*)&size, sizeof(uint));

		// Read the complexity
		uint complexity;
		this->file.read((char*)&complexity, sizeof(uint));

		// Allocate the array
		MlArrayDouble* mlArray = alloc MlArrayDouble(ndim, dims, (MlComplexity)complexity);

		// Read the real data
		this->file.read((char*)mlArray->Re(), size * sizeof(double));

		// If the matrix is complex, read the imaginary data
		if(mxCOMPLEX == (MlComplexity)complexity)
		{
			this->file.read((char*)mlArray->Im(), size * sizeof(double));
		}

		delete[] dims;

		return mlArray;
	}

	MlArray* MlFileBinaryIn::ReadLogical()
	{
		// Read the array dimensions
		uint ndim;
		this->file.read((char*)&ndim, sizeof(uint));

		// Read the dimensions
		uint* dims = alloc uint[ndim];
		for(uint index = 0; index < ndim; index++)
		{
			this->file.read((char*)&dims[index], sizeof(uint));
		}
		
		// Read the array size
		uint size;
		this->file.read((char*)&size, sizeof(uint));

		// Allocate the array
		MlArrayLogical* mlArray = alloc MlArrayLogical(ndim, dims);

		// Read the real data
		this->file.read((char*)mlArray->Logicals(), size * sizeof(bool));

		delete[] dims;

		return mlArray;
	}
}
