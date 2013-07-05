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
#include "MlFileBinary.h"
#include <SimLib/ExceptionUnsupported.h>

using namespace SimLib;

namespace MatLib
{
	MlFileBinary::MlFileBinary()
	{
		// Check the platform is compatible
		if(4 != sizeof(uint)) throw ExceptionUnsupported(__FILE__, __LINE__, "Platform is not supported (size of uint not 4).");
		if(8 != sizeof(double)) throw ExceptionUnsupported(__FILE__, __LINE__, "Platform is not supported (size of double not 8).");
		if(1 != sizeof(char)) throw ExceptionUnsupported(__FILE__, __LINE__, "Platform is not supported (size of char not 1).");
	}
}
