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

// Include

#if defined(_MSC_VER)
	#define _CRT_RAND_S
#endif

#if defined(_MSC_VER) && defined(_DEBUG)
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
#else
	#include <stdlib.h>
#endif

#define _USE_MATH_DEFINES

#include <assert.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <list>
#include <vector>
#include <complex>
#include <string>
#include <queue>
#include <sstream>
#include <algorithm>
#include <limits>

// Definitions
#if defined(__GNUC__)
typedef char						__int8;
typedef short						__int16;
typedef int							__int32;
typedef long long					__int64;
#endif

typedef unsigned int				uint;

typedef double						__bitrate;
typedef double						__time;
typedef unsigned char				__byte;
typedef unsigned long long			__bits;

typedef unsigned char				__uint8;
typedef unsigned short				__uint16;
typedef unsigned int				__uint32;
typedef unsigned long long			__uint64;

#ifdef _DEBUG
#define type_cast					dynamic_cast
#else
#define type_cast					static_cast
#endif

#if defined(_CRTDBG_MAP_ALLOC)
	#define alloc	new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
	#define alloc	new
#endif
