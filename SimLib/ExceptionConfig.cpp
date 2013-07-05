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
#include "ExceptionConfig.h"

namespace SimLib
{
	ExceptionConfig::ExceptionConfig(
		const char*	file,
		uint		line,
		const char*	param,
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

		// Parameter
		if(NULL != param)
		{
			size_t size = strlen(param)+1;
			this->param = alloc char[size];
#ifdef _MSC_VER
			strcpy_s(this->param, size, param);
#else
			strcpy(this->param, param);
#endif
		}
		else this->param = NULL;
	}

	ExceptionConfig::ExceptionConfig(const ExceptionConfig& ex) throw()
		: Exception(ex.file, ex.line)
	{
		if(NULL != ex.param)
		{
			size_t size = strlen(ex.param)+1;
			this->param = alloc char[size];
#ifdef _MSC_VER
			strcpy_s(this->param, size, ex.param);
#else
			strcpy(this->param, ex.param);
#endif
		}
		else this->param = NULL;
	}

	ExceptionConfig::~ExceptionConfig() throw()
	{
		delete[] this->param;
	}

	ExceptionConfigIndex::ExceptionConfigIndex(
		const char*	file,
		uint		line,
		const char*	param,
		uint		index,
		const char*	format,
		...
		) throw() : ExceptionConfig(file, line, param, message)
	{
		// Fixed parameters
		this->index = index;
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
