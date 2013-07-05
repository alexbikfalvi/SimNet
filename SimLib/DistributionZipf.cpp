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
#include "DistributionZipf.h"
#include "Rand.h"

namespace SimLib
{
	DistributionZipf::DistributionZipf(uint n, double s)
	{
		// Parameters
		this->n = n;
		this->s = s;

		// Generalized harmonics
		this->h = alloc double[this->n];
		this->h[0] = 1.0;
		for(uint i = 1; i < n; i++)
			this->h[i] = this->h[i-1] + pow((double)i+1, -this->s);
	}

	DistributionZipf::~DistributionZipf()
	{
		delete[] this->h;
	}

	uint DistributionZipf::operator()()
	{
		double p = Rand::uniform01();

		for(uint k = 1; k <= this->n; k++)
			if(p < this->h[k-1] / this->h[this->n-1])
				return k;
		return this->n;
	}

	double DistributionZipf::Pmf(uint k)
	{
		if(k < 1) return 0.0;
		else if(k > this->n) return 0.0;
		else return pow(k, -this->s) / this->h[this->n-1];
	}

	double DistributionZipf::Cdf(uint k)
	{
		if(k < 1) return 0.0;
		else if(k > this->n) return 1.0;
		else
		{
			return this->h[k-1] / this->h[this->n-1];
		}
	}
}