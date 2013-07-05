/* 
 * Copyright (C) 2012 Alex Bikfalvi
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
#include "Profiler.h"


namespace SimLib
{
	Profiler::Profiler()
	{
#ifdef _MSC_VER
		if(!QueryPerformanceFrequency(&this->frequency)) throw ExceptionUnsupported(__FILE__, __LINE__, "Profiler : QueryPerformanceFrequency is not supported (%u).", GetLastError()); 
#elif __GNUC__
		if(clock_getres(CLOCK_PROCESS_CPUTIME_ID, &this->resolution)) throw ExceptionUnsupported(__FILE__, __LINE__, "Profiler : clock_getres is not supported.");
		this->frequency = 1000000000LL / (1000000000LL * this->resolution.tv_sec + this->resolution.tv_nsec);
#endif
		this->last = 0;
		this->counter = 0;
		this->measurements = 0;
	}
	
	Profiler::~Profiler()
	{
	}

}
