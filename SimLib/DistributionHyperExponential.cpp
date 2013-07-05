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
#include "DistributionHyperExponential.h"
#include "Rand.h"

namespace SimLib
{
	DistributionHyperExponential::DistributionHyperExponential(double l0)
	{
		this->n = 1;

		this->l = alloc double[this->n];
		this->a = alloc double[this->n];

		this->l[0] = l0;
		this->a[0] = 1.0;
	}

	DistributionHyperExponential::DistributionHyperExponential(double l0, double l1, double a0)
	{
		this->n = 2;

		this->l = alloc double[this->n];
		this->a = alloc double[this->n];

		this->l[0] = l0;
		this->l[1] = l1;
		this->a[0] = a0;
		this->a[1] = 1.0 - a0;
	}

	DistributionHyperExponential::DistributionHyperExponential(double l0, double l1, double l2, double a0, double a1)
	{
		this->n = 3;

		this->l = alloc double[this->n];
		this->a = alloc double[this->n];

		this->l[0] = l0;
		this->l[1] = l1;
		this->l[2] = l2;
		this->a[0] = a0;
		this->a[1] = a1;
		this->a[2] = 1.0 - a0 - a1;
	}

	DistributionHyperExponential::DistributionHyperExponential(double l0, double l1, double l2, double l3, double a0, double a1, double a2)
	{
		this->n = 4;

		this->l = alloc double[this->n];
		this->a = alloc double[this->n];

		this->l[0] = l0;
		this->l[1] = l1;
		this->l[2] = l2;
		this->l[3] = l3;
		this->a[0] = a0;
		this->a[1] = a1;
		this->a[2] = a2;
		this->a[3] = 1.0 - a0 - a1 - a2;
	}

	DistributionHyperExponential::DistributionHyperExponential(uint n, double* lambda, double* frac)
	{
		this->n = n;

		this->l = alloc double[this->n];
		this->a = alloc double[this->n];

		for(uint i = 0; i < n; i++)
		{
			this->l[i] = lambda[i];
			this->a[i] = frac[i];
		}
	}

	DistributionHyperExponential::DistributionHyperExponential(const DistributionHyperExponential& object)
	{
		this->n = object.n;

		this->l = alloc double[this->n];
		this->a = alloc double[this->n];

		for(uint i = 0; i < n; i++)
		{
			this->l[i] = object.l[i];
			this->a[i] = object.a[i];
		}
	}

	DistributionHyperExponential::~DistributionHyperExponential()
	{
		delete[] this->l;
		delete[] this->a;
	}

	double DistributionHyperExponential::operator()()
	{
		double prob = Rand::uniform01();
		double sum = this->a[0];
		__uint8 i = 0;

		while((sum < prob) && (i < this->n))
		{
			sum += this->a[++i];
		}
	
		return -log(1 - Rand::uniform01())/this->l[i];
	}

	double DistributionHyperExponential::Pdf(double x)
	{
		if(x < 0.0) return 0.0;
		else
		{
			double f = 0.0;
			for(uint i = 0; i < n; i++)
				f += a[i]*l[i]*exp(-l[i]*x);
			return f;
		}
	}

	double DistributionHyperExponential::Cdf(double x)
	{
		if(x < 0.0) return 0.0;
		else
		{
			double F = 0.0;
			for(uint i = 0; i < n; i++)
				F += a[i]*exp(-l[i]*x);
			return 1.0 - F;
		}
	}
}