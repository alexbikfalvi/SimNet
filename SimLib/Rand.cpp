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
#include "Rand.h"

// If boost library version is greater than or equal to 1.47, all classes of the random library have been moved to boost::random namespace
#if (BOOST_VERSION >= 104700)
	using namespace boost::random;
#else
	using namespace boost;
#endif

namespace SimLib
{

	// Generator
	Rand::UniformGenerator	Rand::generator = Rand::UniformGenerator();

	// Distributions
	uniform_01<double>		Rand::dist01 = uniform_01<double>();

	// Mappings
	Rand::UniformInteger	Rand::uniformInteger = Rand::UniformInteger(Rand::generator);
	Rand::Uniform01			Rand::uniform01 = Rand::Uniform01(Rand::generator, Rand::dist01);

	void Rand::Randomize()
	{
		time_t seed;
		time(&seed);
		Rand::generator.seed((rand48::result_type)seed);
	}
}