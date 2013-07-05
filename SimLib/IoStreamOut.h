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

#include "IoStream.h"

namespace SimLib
{
	class IoStreamOut : public IoStream
	{
	public:
#if defined(_MSC_VER)
		enum Color
		{
			BLACK = 0,
			DARK_BLUE = 1,
			DARK_GREEN = 2,
			DARK_CYAN = 3,
			DARK_RED = 4,
			DARK_MAGENTA = 5,
			DARK_YELLOW = 6,
			DARK_GRAY = 7,
			LIGHT_GRAY = 8,
			LIGHT_BLUE = 9,
			LIGHT_GREEN = 10,
			LIGHT_CYAN = 11,
			LIGHT_RED = 12,
			LIGHT_MAGENTA = 13,
			LIGHT_YELLOW = 14,
			WHITE = 15,
			CLEAR = 16
		};
#else
		enum Color
		{
			BLACK = 30,
			DARK_BLUE = 34,
			DARK_GREEN = 32,
			DARK_CYAN = 36,
			DARK_RED = 31,
			DARK_MAGENTA = 35,
			DARK_YELLOW = 33,
			DARK_GRAY = 30,
			LIGHT_GRAY = 37,
			LIGHT_BLUE = 34,
			LIGHT_GREEN = 32,
			LIGHT_CYAN = 36,
			LIGHT_RED = 31,
			LIGHT_MAGENTA = 35,
			LIGHT_YELLOW = 33,
			WHITE = 37,
			CLEAR = 0
		};
#endif

		IoStreamOut() { }
		virtual ~IoStreamOut() { }

		inline virtual IoStreamOut&	operator <<(const char* value) = 0;
		inline virtual IoStreamOut&	operator <<(char* value) = 0;
		inline virtual IoStreamOut&	operator <<(std::string value) = 0;
		inline virtual IoStreamOut&	operator <<(__int8 value) = 0;
		inline virtual IoStreamOut&	operator <<(__int16 value) = 0;
		inline virtual IoStreamOut&	operator <<(__int32 value) = 0;
		inline virtual IoStreamOut&	operator <<(__int64 value) = 0;
		inline virtual IoStreamOut&	operator <<(__uint8 value) = 0;
		inline virtual IoStreamOut&	operator <<(__uint16 value) = 0;
		inline virtual IoStreamOut&	operator <<(uint value) = 0;
		inline virtual IoStreamOut&	operator <<(__uint64 value) = 0;
		inline virtual IoStreamOut&	operator <<(double value) = 0;
		inline virtual IoStreamOut&	operator <<(Color color) = 0;

#ifdef __GNUC__
		// If the GNU compiler is used, define the operator for a size_t argument
		inline virtual IoStreamOut&	operator <<(size_t value) = 0;
#endif

		inline virtual void			Precision(uint digits) { }
	};
}
