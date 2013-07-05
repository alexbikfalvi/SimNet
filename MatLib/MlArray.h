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

#ifdef _MSC_VER
#include <matrix.h>
#endif

namespace MatLib
{
#ifdef _MSC_VER
	typedef	mwSize			MlSize;
	typedef mxComplexity	MlComplexity;
	typedef mxClassID		MlClassId;
#elif __GNUC__
	typedef uint			MlSize;
	typedef enum
	{
		mxREAL,
		mxCOMPLEX
	}						MlComplexity;
	typedef enum {
        mxUNKNOWN_CLASS,
        mxCELL_CLASS,
        mxSTRUCT_CLASS,
        mxLOGICAL_CLASS,
        mxCHAR_CLASS,
        mxVOID_CLASS,
        mxDOUBLE_CLASS,
        mxSINGLE_CLASS,
        mxINT8_CLASS,
        mxUINT8_CLASS,
        mxINT16_CLASS,
        mxUINT16_CLASS,
        mxINT32_CLASS,
        mxUINT32_CLASS,
        mxINT64_CLASS,
        mxUINT64_CLASS,
        mxFUNCTION_CLASS
	}						MlClassId;
#endif

	class MlArray
	{
	protected:
#ifdef _MSC_VER
		mxArray*		mlArray;
#elif __GNUC__
		MlSize			ndim;
		MlSize			size;
		MlSize*			dims;
		MlClassId		classId;
#endif

	public:
		MlArray(MlSize ndim = 0, const MlSize* dims = NULL);
		MlArray(const MlArray& obj);
		virtual ~MlArray();

#ifdef _MSC_VER
		inline MlSize			NumberOfDimensions() { return mxGetNumberOfDimensions(this->mlArray); }
		inline MlSize			NumberOfElements() { return mxGetNumberOfElements(this->mlArray); }
		inline const MlSize*	Dimensions() { return mxGetDimensions(this->mlArray); }
		inline MlClassId		ClassId() { return mxGetClassID(this->mlArray); }

		inline mxArray*			Array() { return this->mlArray; }
#elif __GNUC__
		inline MlSize			NumberOfDimensions() { return this->ndim; }
		inline MlSize			NumberOfElements() { return this->size; }
		inline const MlSize*	Dimensions() { return this->dims; }
		inline MlClassId		ClassId() { return this->classId; }
#endif
	};
}
