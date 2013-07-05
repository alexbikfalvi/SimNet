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
#include "FitNormal.h"
#include "ExceptionUnsupported.h"
#include <boost/math/special_functions/gamma.hpp>

namespace SimLib
{
	FitNormal::FitNormal(double* data, uint size)
	{
		for(uint index = 0; index < size; index++)
			this->Add(data[index]);
	}

	FitNormal::FitNormal(std::vector<double>& data)
	{
		for(std::vector<double>::iterator iter = data.begin(); iter != data.end(); iter++)
			this->Add(*iter);
	}

	void FitNormal::Add(double value)
	{
		this->moment.Add(value);
	}

	void FitNormal::Remove()
	{
		this->moment.Remove();
	}

	bool FitNormal::TestChiSquare(double& pValue, double significance)
	{
		pValue = std::numeric_limits<double>::quiet_NaN(); // Undefined p-value

		double b1 = this->moment.Skewness();
		double b2 = this->moment.Kurtosis();
		double n = this->moment.Count();

		// Return true if the number of samples is less than 20
		if(n < 20.0) return true;

		// Test of skewness
		double Y = b1*sqrt((n+1.0)*(n+2.0)/(6.0*(n-2.0)));
		double beta2 = 3.0*(n*n+27.0*n-70.0)*(n+1.0)*(n+3.0)/((n-2.0)*(n+5.0)*(n+7.0)*(n+9.0));
		double W2 = -1.0 + sqrt(2.0*(beta2-1.0));
		double W = sqrt(W2);
		double delta = 1.0/sqrt(log(W));
		double alpha = sqrt(2.0/(W2-1.0));
		double Zb1 = delta*log(Y/alpha + sqrt(pow(Y/alpha,2)+1.0));

		// Test of kurtosis
		double Eb2 = 3.0*(n-1.0)/(n+1.0);
		double varb2 = 24.0*n*(n-2.0)*(n-3.0)/((pow(n+1.0,2))*(n+3.0)*(n+5.0));
		double x = (b2-Eb2)/sqrt(varb2);
		double beta1 = 6.0*(n*n-5.0*n+2.0)*sqrt(6.0*(n+3.0)*(n+5.0)/(n*(n-2.0)*(n-3.0)))/((n+7.0)*(n+9.0));
		double A = 6.0 + 8.0*(2.0/beta1 + sqrt(1.0+4.0/beta1))/beta1;
		double Zb2 = ((1.0-2.0/(9.0*A)) - pow((1.0-2.0/A)/(1.0+x*sqrt(2.0/(A-4.0))),1.0/3.0))/sqrt(2.0/(9.0*A));

		// Omnibus test
		double K2 = -(Zb1*Zb1 + Zb2*Zb2)/2.0;

		pValue = std::exp(K2);

		// If pValue is NaN
		if(pValue != pValue) return true;
		else return pValue <= significance;
	}
}