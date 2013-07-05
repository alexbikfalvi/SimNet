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
#include "FilterWindow.h"

namespace SimLib
{
	FilterWindow::FilterWindow(uint window, double ratio)
	{
		this->window = window;
		this->ratio = ratio;
	}

	double FilterWindow::operator()(double* input, double* output, uint index, uint size)
	{
		return ((index > 0) ? output[index-1] : 0.0) + (input[index] - ((index > this->window) ? input[index-this->window-1] : 0.0 )) / this->ratio;
	}

	double FilterWindow::operator()(std::vector<double>& input, std::vector<double>& output, uint index)
	{
		return ((index > 0) ? output[index-1] : 0.0) + (input[index] - ((index > this->window) ? input[index-this->window-1] : 0.0 )) / this->ratio;
	}
}
