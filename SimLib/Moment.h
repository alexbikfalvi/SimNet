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
	class Moment
	{
	private:
		uint				sum0;
		double				sum1;
		double				sum2;
		double				sum3;
		double				sum4;

		double				mean;
		double				varB;
		double				varU;
		double				std;
		double				skew;
		double				kurt;

		std::queue<double>	samples;

	public:
		Moment() : sum0(0), sum1(0.0), sum2(0.0), sum3(0.0), sum4(0.0) { }
		virtual ~Moment() { }

		void			Add(double value);
		void			Remove();

		void			Clear();

		inline uint		Count() { return this->sum0; }
		inline double	Mean() { return this->mean; }
		inline double	Variance() { return this->varU; }
		inline double	StdDev() { return this->std; }
		inline double	Skewness() { return this->skew; }
		inline double	Kurtosis() { return this->kurt; }

		static Moment	Of(double* data, uint size);
		static Moment	Of(double* data, uint begin, uint end);
		static Moment	Of(std::vector<double>& data);
		static Moment	Of(std::vector<double>& data, uint begin, uint end);

	protected:
		void			operator +=(double value);
		void			operator -=(double value);
	};
}
