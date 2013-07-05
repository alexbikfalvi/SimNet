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

namespace SimLib
{
	class Hypergeometric2F1
	{
	private:
		double a;
		double b;
		double c;

		double c1;
		double c2;
		double c3;
		double c4;
		double c5;
		double c6;
		double c7;
		double c8;
		double c9;
		double c10;
		double c11;
		double c12;

	public:
		Hypergeometric2F1(double a, double b, double c);

		~Hypergeometric2F1() { }

		double					Real(double z);
		std::complex<double>	Complex(double z);
	};
}
