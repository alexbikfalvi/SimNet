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

#include "Moment.h"

namespace SimLib
{
	class FitNormal
	{
	private:
		Moment	moment;

	public:
		FitNormal() { }
		FitNormal(double* data, uint size);
		FitNormal(std::vector<double>& data);
		virtual ~FitNormal() { }

		void			Add(double value);
		void			Remove();

		inline uint		Count() { return this->moment.Count(); }
		inline void		Clear() { this->moment.Clear(); }

		inline Moment&	Mom() { return this->moment; }
		
		//
		// Chi-square omnibus fit test: if returns true, the null hypothesis can be rejected at the given significance level
		//
		bool			TestChiSquare(double& pValue, double significance = 0.05);
	};
}
