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
#include "Dft.h"

namespace SimLib
{
	Dft::Dft(uint n)
	{
		this->n = n;

		this->cd = alloc std::complex<double>[this->n];
		this->ci = alloc std::complex<double>[this->n];

		double arg = 2*M_PI/this->n;
		for(uint i = 0; i < n; i++)
		{
			this->cd[i] = exp(std::complex<double>(0, -arg*i));
			this->ci[i] = exp(std::complex<double>(0, arg*i))/(double)this->n;
		}
	}

	Dft::~Dft()
	{
		delete[] this->cd;
		delete[] this->ci;
	}

	void Dft::Direct(double* in, std::complex<double>* out)
	{
		for(uint i = 0; i < this->n; i++)
		{
			out[i] = std::complex<double>(0,0);
//			for(uint j = 0; j < this->n; j++)
//				out[i] += in[j];//*this->cd[j];
		}
	}

	void Dft::Direct(std::complex<double>* in, std::complex<double>* out)
	{
		for(uint i = 0; i < this->n; i++)
		{
			out[i] = std::complex<double>(0,0);
			for(uint j = 0; j < this->n; j++)
				out[i] += in[j]*this->cd[j];
		}
	}

	void Dft::Inverse(std::complex<double>* in, std::complex<double>* out)
	{
		for(uint i = 0; i < this->n; i++)
		{
			out[i] = 0;
			for(uint j = 0; j < this->n; j++)
				out[i] += in[j]*this->ci[j];
		}
	}
}