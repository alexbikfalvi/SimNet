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

#pragma once

#include <SimLib/IoStreamOut.h>

#ifdef _MSC_VER
#include <mex.h>
#endif

namespace MatLib
{
	using namespace SimLib;

	class IoStreamOutMatlab : public IoStreamOut
	{
	public:
		IoStreamOutMatlab() { }
		virtual ~IoStreamOutMatlab() { }

#ifdef _MSC_VER                
		inline virtual IoStreamOut&	operator <<(const char* value) { mexPrintf("%s", value); return *this; }
		inline virtual IoStreamOut&	operator <<(char* value) { mexPrintf("%s", value); return *this; }
		inline virtual IoStreamOut&	operator <<(std::string value) { mexPrintf("%s", value.c_str()); return *this; }
		inline virtual IoStreamOut&	operator <<(__int8 value) { mexPrintf("%d", value); return *this; }
		inline virtual IoStreamOut&	operator <<(__int16 value) { mexPrintf("%d", value); return *this; }
		inline virtual IoStreamOut&	operator <<(__int32 value) { mexPrintf("%d", value); return *this; }
		inline virtual IoStreamOut&	operator <<(__int64 value) { mexPrintf("%lld", value); return *this; }
		inline virtual IoStreamOut&	operator <<(__uint8 value) { mexPrintf("%u", value); return *this; }
		inline virtual IoStreamOut&	operator <<(__uint16 value) { mexPrintf("%u", value); return *this; }
		inline virtual IoStreamOut&	operator <<(__uint32 value) { mexPrintf("%u", value); return *this; }
		inline virtual IoStreamOut&	operator <<(__uint64 value) { mexPrintf("%llu", value); return *this; }
		inline virtual IoStreamOut&	operator <<(double value) { mexPrintf("%lf", value); return *this; }
		inline virtual IoStreamOut&	operator <<(Color color) { return *this; }

#elif __GNUC__
		// If the GNU compiler is used, define the operator for a size_t argument
		inline virtual IoStreamOut&	operator <<(const char* value) { printf("%s", value); return *this; }
		inline virtual IoStreamOut&	operator <<(char* value) { printf("%s", value); return *this; }
		inline virtual IoStreamOut&	operator <<(std::string value) { printf("%s", value.c_str()); return *this; }
		inline virtual IoStreamOut&	operator <<(__int8 value) { printf("%d", value); return *this; }
		inline virtual IoStreamOut&	operator <<(__int16 value) { printf("%d", value); return *this; }
		inline virtual IoStreamOut&	operator <<(__int32 value) { printf("%d", value); return *this; }
		inline virtual IoStreamOut&	operator <<(__int64 value) { printf("%lld", value); return *this; }
		inline virtual IoStreamOut&	operator <<(__uint8 value) { printf("%u", value); return *this; }
		inline virtual IoStreamOut&	operator <<(__uint16 value) { printf("%u", value); return *this; }
		inline virtual IoStreamOut&	operator <<(__uint32 value) { printf("%u", value); return *this; }
		inline virtual IoStreamOut&	operator <<(__uint64 value) { printf("%llu", value); return *this; }
		inline virtual IoStreamOut&	operator <<(double value) { printf("%lf", value); return *this; }
		inline virtual IoStreamOut&	operator <<(size_t value) { printf("%lu", value); return *this; }
		inline virtual IoStreamOut&	operator <<(Color color) { return *this; }
#endif
                
		inline virtual void			Precision(uint digits) { }
		
		static IoStreamOutMatlab		stream;
	};
}

