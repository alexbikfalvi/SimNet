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

namespace SimLib
{
	//
	// Calculate the biased and unbiased sample variance (with Bessel correction) of a data set
	//
	class Variance
	{
	private:
		uint	sum0;
		double	sum1;
		double	sum2;

	public:
		Variance() : sum0(0), sum1(0), sum2(0) { }
		virtual ~Variance() { }

		void			operator +=(double x);
		void			operator -=(double x);
		inline double	operator ()() { return this->Unbiased(); }

		void			Clear();
		
		double			Biased();
		double			Unbiased();
		
		static double	Of(double* data, uint count) { return Variance::OfUnbiased(data, count); }
		static double	Of(double* data, uint begin, uint end) { return Variance::OfUnbiased(data,  begin, end); }

		static double	Of(std::vector<double>& data) { return Variance::OfUnbiased(data); }
		static double	Of(std::vector<double>& data, uint begin, uint end) { return Variance::OfUnbiased(data, begin, end); }

		static double	OfNan(double* data, uint count) { return Variance::OfUnbiasedNan(data, count); }
		static double	OfNan(double* data, uint begin, uint end) { return Variance::OfUnbiasedNan(data,  begin, end); }

		static double	OfNan(std::vector<double>& data) { return Variance::OfUnbiasedNan(data); }
		static double	OfNan(std::vector<double>& data, uint begin, uint end) { return Variance::OfUnbiasedNan(data, begin, end); }

		static double	OfBiased(double* data, uint count);
		static double	OfBiased(double* data, uint begin, uint end);

		static double	OfBiased(std::vector<double>& data);
		static double	OfBiased(std::vector<double>& data, uint begin, uint end);
		
		static double	OfBiasedNan(double* data, uint count);
		static double	OfBiasedNan(double* data, uint begin, uint end);

		static double	OfBiasedNan(std::vector<double>& data);
		static double	OfBiasedNan(std::vector<double>& data, uint begin, uint end);
		
		static double	OfUnbiased(double* data, uint count);
		static double	OfUnbiased(double* data, uint begin, uint end);

		static double	OfUnbiased(std::vector<double>& data);
		static double	OfUnbiased(std::vector<double>& data, uint begin, uint end);
		
		static double	OfUnbiasedNan(double* data, uint count);
		static double	OfUnbiasedNan(double* data, uint begin, uint end);

		static double	OfUnbiasedNan(std::vector<double>& data);
		static double	OfUnbiasedNan(std::vector<double>& data, uint begin, uint end);
	};
}
