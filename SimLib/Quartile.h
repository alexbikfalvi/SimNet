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
	class Quartile
	{
	private:
		std::vector<double>		copy;
		std::vector<double>&	data;

		double					first;
		double					second;
		double					third;
		double					min;
		double					max;
		double					mean;
		double					iqr;
		double					fenceLo;
		double					fenceHi;

		std::vector<double>		outliers;

	public:
		Quartile(std::vector<double>& data);
		Quartile(std::vector<double>& data, uint begin, uint end);
		Quartile(double* data, uint count);
		virtual ~Quartile() { }

		inline double				First() { return this->first; }
		inline double				Second() { return this->second; }
		inline double				Third() { return this->third; }
		inline double				Min() { return this->min; }
		inline double				Max() { return this->max; }
		inline double				Mean() { return this->mean; }
		inline double				Iqr() { return this->iqr; }
		inline double				FenceLo() { return this->fenceLo; }
		inline double				FenceHi() { return this->fenceHi; }
		inline std::vector<double>&	Outliers() { return this->outliers; }
	private:
		double						Median(uint begin, uint end, uint& midLo, uint& midHi);
	};
}
