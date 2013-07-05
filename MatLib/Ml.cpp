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
#include "Ml.h"

namespace MatLib
{
	MlArrayDouble* Ml::MlCreateDoubleScalar(double value)
	{
		MlSize dims[1] = {1};
		return alloc MlArrayDouble(1, dims, mxREAL, &value);
	}

	MlArrayDouble* Ml::MlCreateDoubleMatrix(MlSize m, MlSize n, MlComplexity complexity)
	{
		MlSize dims[2] = {m, n};
		return alloc MlArrayDouble(2, dims, complexity);
	}

	MlArrayDouble* Ml::MlCreateDoubleMatrix(MlSize m, MlSize n, std::vector<double>& re)
	{
		MlSize dims[2] = {m, n};
		return alloc MlArrayDouble(2, dims, re);
	}

	MlArrayDouble* Ml::MlCreateDoubleArray(MlSize ndim, const MlSize* dims, MlComplexity complexity)
	{
		return alloc MlArrayDouble(ndim, dims, complexity);
	}

	MlArrayLogical* Ml::MlCreateLogicalScalar(bool value)
	{
		MlSize dims[1] = {1};
		return alloc MlArrayLogical(1, dims, &value);
	}

	MlArrayLogical* Ml::MlCreateLogicalMatrix(MlSize m, MlSize n)
	{
		MlSize dims[2] = {m, n};
		return alloc MlArrayLogical(2, dims);
	}

	MlArrayLogical* Ml::MlCreateLogicalArray(MlSize ndim, const MlSize* dims)
	{
		return alloc MlArrayLogical(ndim, dims);
	}

	void Ml::MlDestroyArray(MlArray* pa)
	{
		delete pa;
	}
}
