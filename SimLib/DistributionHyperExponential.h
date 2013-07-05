#pragma once

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

#include "DistributionContinuous.h"
#include "ExceptionUnsupported.h"

namespace SimLib
{
	class DistributionHyperExponential : public DistributionContinuous
	{
	private:
		double*	l;
		double*	a;
		uint	n;

	public:
		DistributionHyperExponential(double lambda0);
		DistributionHyperExponential(double lambda0, double lambda1, double frac0);
		DistributionHyperExponential(double lambda0, double lambda1, double lambda2, double frac0, double frac1);
		DistributionHyperExponential(double lambda0, double lambda1, double lambda2, double lambda3, double frac0, double frac1, double frac2);
		DistributionHyperExponential(uint n, double* lambda, double* frac);
		DistributionHyperExponential(const DistributionHyperExponential& object);

		virtual ~DistributionHyperExponential();

		virtual double	operator()();
		virtual double	Pdf(double x);
		virtual double	Cdf(double x);
	};
}
