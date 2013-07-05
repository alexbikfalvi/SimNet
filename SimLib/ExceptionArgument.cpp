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
#include "ExceptionArgument.h"

namespace SimLib
{
	ExceptionArgument::ExceptionArgument(
		const char*	file,
		uint		line,
		const char*	format,
		...
		) throw() : Exception(file, line)
	{
		// Variable arguments
		va_list args;
		va_start(args, format);
#if defined(_MSC_VER)
		vsprintf_s(this->message, EXCEPTION_MESSAGE_SIZE*sizeof(char), format, args);
#else
		vsnprintf(this->message, EXCEPTION_MESSAGE_SIZE, format, args);
#endif
		va_end(args);
	}
}
