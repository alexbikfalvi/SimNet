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

#include "MlArray.h"

namespace MatLib
{
	class MlArrayDouble : public MlArray
	{
	private:
#if __GNUC__
		MlComplexity	complexity;
		double*			re;
		double*			im;
#endif

	public:
		MlArrayDouble(MlSize ndim, const MlSize* dims, MlComplexity complexity = mxREAL, double* re = NULL, double* im = NULL);
		MlArrayDouble(MlSize ndim, const MlSize* dims, std::vector<double>& re);
		MlArrayDouble(const MlArrayDouble& obj);
		virtual ~MlArrayDouble();

#ifdef _MSC_VER
		inline MlComplexity		Complexity() { return mxIsComplex(this->mlArray) ? mxCOMPLEX : mxREAL; }
		inline double*			Re() { return mxGetPr(this->mlArray); }
		inline double*			Im() { return mxGetPi(this->mlArray); }
#elif __GNUC__
		inline MlComplexity		Complexity() { return this->complexity; }
		inline double*			Re() { return this->re; }
		inline double*			Im() { return this->im; }
#endif
	};
}
