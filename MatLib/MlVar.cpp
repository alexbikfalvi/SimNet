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
#include "MlVar.h"
#include <SimLib/ExceptionArgument.h>

using namespace SimLib;

namespace MatLib
{
	MlVar::MlVar(const char* name, MlArray* mlArray)
	{
		if(NULL == name) throw ExceptionArgument(__FILE__, __LINE__, "Variable name cannot be null.");

		size_t size = strlen(name)+1;
		this->name = alloc char[size];
#if _MSC_VER
		strcpy_s(this->name, size, name);
#else
		strcpy(this->name, name);
#endif
		this->mlArray = mlArray;
	}

	MlVar::MlVar(const MlVar& obj)
	{
		size_t size = strlen(obj.name)+1;
		this->name = alloc char[size];
#if _MSC_VER
		strcpy_s(this->name, size, obj.name);
#else
		strcpy(this->name, obj.name);
#endif
		this->mlArray = obj.mlArray;
	}

	MlVar::~MlVar()
	{
		delete[] this->name;
	}
}
