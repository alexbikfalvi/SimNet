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
#include "Ml.h"
#include "MlFileOut.h"
#include <SimLib/ExceptionIo.h>
#include <SimLib/ExceptionArgument.h>
#include <SimLib/ExceptionUnsupported.h>

using namespace SimLib;

namespace MatLib
{
#ifdef _MSC_VER
	MlFileOut::MlFileOut(const char* fileName)
	{
		if(NULL == fileName) throw ExceptionArgument(__FILE__, __LINE__, "MlFileOut::ctor : file name cannot be null.");
		if(NULL == (this->file = matOpen(fileName, "w"))) throw ExceptionIo(__FILE__, __LINE__, "Opening the file failed.");
	}
#endif

	MlFileOut::~MlFileOut()
	{
#ifdef _MSC_VER
		if(matClose(this->file)) throw ExceptionIo(__FILE__, __LINE__, "Closing the file failed.");
#endif
	}

	void MlFileOut::Write(const char* name, MlArray* mlArray)
	{
		if(NULL == name) throw ExceptionArgument(__FILE__, __LINE__, "MlFileOut::Write : Variable name cannot be null.");
		if(NULL == mlArray) throw ExceptionArgument(__FILE__, __LINE__, "MlFileOut::Write : Array pointer cannot be null.");
		switch(mlArray->ClassId())
		{
		case mxDOUBLE_CLASS: this->Write(name, type_cast<MlArrayDouble*>(mlArray)); break;
		case mxLOGICAL_CLASS: this->Write(name, type_cast<MlArrayLogical*>(mlArray)); break;
		default: throw ExceptionUnsupported(__FILE__, __LINE__, "MlFileOut::Write : Matlab array class %u is not supported.", mlArray->ClassId());
		}
	}

	void MlFileOut::Write(const char* name, MlArrayDouble* mlArray)
	{
		if(NULL == name) throw ExceptionArgument(__FILE__, __LINE__, "MlFileOut::Write : Variable name cannot be null.");
		if(NULL == mlArray) throw ExceptionArgument(__FILE__, __LINE__, "MlFileOut::Write : Array pointer cannot be null.");
#ifdef _MSC_VER
		if(matPutVariable(this->file, name, mlArray->Array())) throw ExceptionIo(__FILE__, __LINE__, "Writing to the file failed.");
#elif __GNUC__
		this->file.Write(name, mlArray);
#endif
	}

	void MlFileOut::Write(const char* name, MlArrayLogical* mlArray)
	{
		if(NULL == name) throw ExceptionArgument(__FILE__, __LINE__, "MlFileOut::Write : Variable name cannot be null.");
		if(NULL == mlArray) throw ExceptionArgument(__FILE__, __LINE__, "MlFileOut::Write : Array pointer cannot be null.");
#ifdef _MSC_VER
		if(matPutVariable(this->file, name, mlArray->Array())) throw ExceptionIo(__FILE__, __LINE__, "Writing to the file failed.");
#elif __GNUC__
		this->file.Write(name, mlArray);
#endif
	}
	void MlFileOut::Write(const char* name, double value)
	{
		if(NULL == name) throw ExceptionArgument(__FILE__, __LINE__, "MlFileOut::Write : Variable name cannot be null.");

		MlArrayDouble* mlArray = Ml::MlCreateDoubleScalar(value);

#ifdef _MSC_VER
		if(matPutVariable(this->file, name, mlArray->Array())) throw ExceptionIo(__FILE__, __LINE__, "Writing to the file failed.");
#elif __GNUC__
		this->file.Write(name, mlArray);
#endif

		Ml::MlDestroyArray(mlArray);
	}
}
