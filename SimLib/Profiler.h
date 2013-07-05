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

#pragma once

#ifdef _MSC_VER
#include <windows.h>
#elif __GNUC__
#include <time.h>
#endif

#include "ExceptionUnsupported.h"

namespace SimLib
{
	class Profiler
	{
	private:
#ifdef _MSC_VER
		LARGE_INTEGER	counterOverhead;
		LARGE_INTEGER	counterBegin;
		LARGE_INTEGER	counterEnd;
		LARGE_INTEGER	frequency;
		__uint64		counter;
		__uint64		last;

#elif __GNUC__
		struct timespec	resolution;
		struct timespec	timerBegin;
		struct timespec	timerEnd;

		__uint64		counterBegin;
		__uint64		counterEnd;
		__uint64		last;
		__uint64		counter;
		__uint64		frequency;
#endif
		uint			measurements;

	public:
		Profiler();
		~Profiler();

#ifdef _MSC_VER
		inline void		Begin()
		{
			if(!QueryPerformanceCounter(&this->counterBegin)) throw ExceptionUnsupported(__FILE__, __LINE__, "Profiler : QueryPerformanceCounter is not supported (%u)", GetLastError()); 
		}
		inline void		End()
		{
			if(!QueryPerformanceCounter(&this->counterEnd)) throw ExceptionUnsupported(__FILE__, __LINE__, "Profiler : QueryPerformanceCounter is not supported (%u)", GetLastError()); 

			this->last = this->counterEnd.QuadPart - this->counterBegin.QuadPart;
			this->counter += this->last;
			this->measurements++;
		}

		inline __uint64	Counter() { return this->counter; }
		inline __uint64	Frequency() { return this->frequency.QuadPart; }
		inline __time	Delay() { return ((__time)this->last) / this->frequency.QuadPart; }

#elif __GNUC__
		inline void		Begin()
		{
			if(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &this->timerBegin)) throw ExceptionUnsupported(__FILE__, __LINE__, "Profiler : clock_gettime is not supported.");
		}
		inline void		End()
		{
			if(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &this->timerEnd)) throw ExceptionUnsupported(__FILE__, __LINE__, "Profiler : clock_gettime is not supported.");

			this->counterBegin = 1000000000LL * this->timerBegin.tv_sec + this->timerBegin.tv_nsec;
			this->counterEnd = 1000000000LL * this->timerEnd.tv_sec + this->timerEnd.tv_nsec;

			this->last = this->counterEnd - this->counterBegin;
			this->counter += this->last;
			this->measurements++;
		}

		inline __uint64	Counter() { return this->counter; }
		inline __uint64	Frequency() { return this->frequency; }
		inline __time	Delay() { return ((__time)this->last) / 1000000000.0; }

#endif
		inline uint		Measurements() { return this->measurements; }
	};
}
