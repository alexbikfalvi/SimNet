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
#include "ErlangBSolve.h"

namespace SimLib
{
	uint ErlangBSolve(double b, double a)
	{
		double B = 1;
		uint k = 1;
		while(B > b)
		{
			B = a*B/(++k + a*B);
		}
		return k;
	}

	void ErlangBSolve(double b, uint count, double* a, uint* k)
	{
		double B;
		for(uint i = 0; i < count; i++)
		{
			B = 1;
			k[i] = 1;
			while(B > b)
			{
				B = a[i]*B/(++k[i] + a[i]*B);
			}
		}
	}
}
