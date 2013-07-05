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
#include "ErlangB.h"

namespace SimLib
{
	double	ErlangB(uint k, double a)
	{
		double b = 1;
		for(uint i = 1; i <= k; i++)
		{
			b = (a*b)/(i+a*b);
		}
		return b;
	}

	void	ErlangB(uint* k, uint countK, double* a, uint countA, double* b)
	{
		uint m;
		for(uint i = 0; i < countK; i++)
		{
			for(uint j = 0; j < countA; j++)
			{
				m = i*countA+j; 
				b[m] = 1;
				for(uint n = 1; n <= k[i]; n++)
				{
					b[m] = (a[j]*b[m])/(n+a[j]*b[m]);
				}
			}
		}
	}
}
