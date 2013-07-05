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
#include "MlFileBinaryOut.h"
#include <SimLib/ExceptionIo.h>
#include <SimLib/ExceptionArgument.h>

using namespace SimLib;

namespace MatLib
{
	MlFileBinaryOut::MlFileBinaryOut(const char* fileName)
	{
		this->file.open(fileName, std::ios::binary | std::ios::out);
		if(!this->file.good()) throw ExceptionIo(__FILE__, __LINE__, "Opening the binary file failed.");
	}

	MlFileBinaryOut::~MlFileBinaryOut()
	{
		this->file.close();
	}

	void MlFileBinaryOut::Write(const char* name, MlArrayDouble* mlArray)
	{
		// Write the array to the file

		// Check the file name
		if(NULL == name) throw ExceptionArgument(__FILE__, __LINE__, "Array name cannot be null.");

		// Move to the end
		this->file.seekp(0, std::ios::end);

		// Get the name size
		uint nameSize = strlen(name) + 1;

		uint tmpUint;

		// Write the name size
		this->file.write((const char*)&nameSize, sizeof(uint));
		// Write the name
		this->file.write(name, nameSize * sizeof(char));
		
		// Write the class ID
		tmpUint = mlArray->ClassId();
		this->file.write((const char*)&tmpUint, sizeof(uint));

		// Write the array dimensions
		tmpUint = mlArray->NumberOfDimensions();
		this->file.write((const char*)&tmpUint, sizeof(uint));

		for(uint index = 0; index < mlArray->NumberOfDimensions(); index++)
		{
			tmpUint = mlArray->Dimensions()[index];
			this->file.write((const char*)&tmpUint, sizeof(uint));
		}
		
		// Write the array size
		tmpUint = mlArray->NumberOfElements();
		this->file.write((const char*)&tmpUint, sizeof(uint));

		// Write the array complexity
		tmpUint = mlArray->Complexity();
		this->file.write((const char*)&tmpUint, sizeof(uint));

		// Write the real data
		this->file.write((const char*)mlArray->Re(), mlArray->NumberOfElements() * sizeof(double));

		// If array is complex, write the complex data
		if(mlArray->Complexity() == mxCOMPLEX)
		{
			this->file.write((const char*)mlArray->Im(), mlArray->NumberOfElements() * sizeof(double));
		}
	}

	void MlFileBinaryOut::Write(const char* name, MlArrayLogical* mlArray)
	{
		// Write the array to the file

		// Check the file name
		if(NULL == name) throw ExceptionArgument(__FILE__, __LINE__, "Array name cannot be null.");

		// Move to the end
		this->file.seekp(0, std::ios::end);

		// Get the name size
		uint nameSize = strlen(name) + 1;

		uint tmpUint;

		// Write the name size
		this->file.write((const char*)&nameSize, sizeof(uint));
		// Write the name
		this->file.write(name, nameSize * sizeof(char));
		
		// Write the class ID
		tmpUint = mlArray->ClassId();
		this->file.write((const char*)&tmpUint, sizeof(uint));

		// Write the array dimensions
		tmpUint = mlArray->NumberOfDimensions();
		this->file.write((const char*)&tmpUint, sizeof(uint));

		for(uint index = 0; index < mlArray->NumberOfDimensions(); index++)
		{
			tmpUint = mlArray->Dimensions()[index];
			this->file.write((const char*)&tmpUint, sizeof(uint));
		}
		
		// Write the array size
		tmpUint = mlArray->NumberOfElements();
		this->file.write((const char*)&tmpUint, sizeof(uint));

		// Write the logical data
		this->file.write((const char*)mlArray->Logicals(), mlArray->NumberOfElements() * sizeof(bool));
	}
}
