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
#include "SignalRectangular.h"
#include "ExceptionArgument.h"

namespace SimLib
{
	SignalRectangular::SignalRectangular(double period, double lo, double hi)
	{
		if(0.0 == period) throw ExceptionArgument(__FILE__, __LINE__, "SignalRectangular::ctor : period cannot be zero.");

		this->period = period;
		this->periodHalf = this->period / 2.0;
		this->lo = lo;
		this->hi = hi;
	}

	double SignalRectangular::operator()(uint time)
	{
		double t = fmod(time, this->period);
		return (t < this->periodHalf) ?  this->hi : this->lo ;
	}
}
