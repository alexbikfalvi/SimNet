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
	class FitEmpirical
	{
	private:
		std::multiset<double>	data;
		std::vector<double>		vdata;
		std::vector<double>		cdf;
		std::queue<double>		samples;
	
	public:
		FitEmpirical() { }
		FitEmpirical(double* data, uint size);
		FitEmpirical(std::vector<double>& data);
		virtual ~FitEmpirical() { }

		void					Clear();

		void					Add(double value);
		void					Remove();
		inline uint				Count() { return this->samples.size(); }
		
		double					Min();
		double					Max();
		double					Quantile(double prob);
		std::vector<double>&	Data();
		std::vector<double>&	Cdf();
	};
}
