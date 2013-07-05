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

/*
#define RAND_DET_MINSTD_RAND0			// Deterministic pseudo-random generator (http://www.boost.org/doc/libs/1_46_1/doc/html/boost/minstd_rand0.html)
#define RAND_DET_MINSTD_RAND			// Deterministic pseudo-random generator (http://www.boost.org/doc/libs/1_46_1/doc/html/boost/minstd_rand.html)
*/
#define RAND_DET_RAND48					// Deterministic pseudo-random generator (http://www.boost.org/doc/libs/1_46_1/doc/html/boost/rand48.html)
/*
#define RAND_DET_ECUYER1988				// Deterministic pseudo-random generator (http://www.boost.org/doc/libs/1_46_1/doc/html/boost/ecuyer1988.html)
#define RAND_DET_KREUTZER1986			// Deterministic pseudo-random generator (http://www.boost.org/doc/libs/1_46_1/doc/html/boost/kreutzer1986.html)
#define RAND_DET_TAUS88					// Deterministic pseudo-random generator (http://www.boost.org/doc/libs/1_46_1/doc/html/boost/taus88.html)
#define RAND_DET_HELLEKALEK1995			// Deterministic pseudo-random generator (http://www.boost.org/doc/libs/1_46_1/doc/html/boost/hellekalek1995.html)
#define RAND_DET_MT11213B				// Deterministic pseudo-random generator (http://www.boost.org/doc/libs/1_46_1/doc/html/boost/mt11213b.html)
#define RAND_DET_MT19937				// Deterministic pseudo-random generator (http://www.boost.org/doc/libs/1_46_1/doc/html/boost/mt19937.html)
#define RAND_DET_LAGGED_FIBONACCI607	// Deterministic pseudo-random generator (http://www.boost.org/doc/libs/1_46_1/doc/html/boost/lagged_fibonacci607.html)
#define RAND_DET_LAGGED_FIBONACCI1279	// Deterministic pseudo-random generator (http://www.boost.org/doc/libs/1_46_1/doc/html/boost/lagged_fibonacci1279.html)
#define RAND_DET_LAGGED_FIBONACCI2281	// Deterministic pseudo-random generator (http://www.boost.org/doc/libs/1_46_1/doc/html/boost/lagged_fibonacci2281.html)
#define RAND_DET_LAGGED_FIBONACCI3217	// Deterministic pseudo-random generator (http://www.boost.org/doc/libs/1_46_1/doc/html/boost/lagged_fibonacci3217.html)
#define RAND_DET_LAGGED_FIBONACCI4423	// Deterministic pseudo-random generator (http://www.boost.org/doc/libs/1_46_1/doc/html/boost/lagged_fibonacci4423.html)
#define RAND_DET_LAGGED_FIBONACCI9689	// Deterministic pseudo-random generator (http://www.boost.org/doc/libs/1_46_1/doc/html/boost/lagged_fibonacci9689.html)
#define RAND_DET_LAGGED_FIBONACCI19937	// Deterministic pseudo-random generator (http://www.boost.org/doc/libs/1_46_1/doc/html/boost/lagged_fibonacci19937.html)
#define RAND_DET_LAGGED_FIBONACCI23209	// Deterministic pseudo-random generator (http://www.boost.org/doc/libs/1_46_1/doc/html/boost/lagged_fibonacci23209.html)
#define RAND_DET_LAGGED_FIBONACCI44497	// Deterministic pseudo-random generator (http://www.boost.org/doc/libs/1_46_1/doc/html/boost/lagged_fibonacci44497.html)
#define RAND_DET_RANLUX3				// Deterministic pseudo-random generator (http://www.boost.org/doc/libs/1_46_1/doc/html/boost/ranlux3.html)
#define RAND_DET_RANLUX4				// Deterministic pseudo-random generator (http://www.boost.org/doc/libs/1_46_1/doc/html/boost/ranlux4.html)
#define RAND_DET_RANLUX64_3				// Deterministic pseudo-random generator (http://www.boost.org/doc/libs/1_46_1/doc/html/boost/ranlux64_3.html)
#define RAND_DET_RANLUX64_4				// Deterministic pseudo-random generator (http://www.boost.org/doc/libs/1_46_1/doc/html/boost/ranlux64_4.html)
#define RAND_DET_RANLUX3_01				// Deterministic pseudo-random generator (http://www.boost.org/doc/libs/1_46_1/doc/html/boost/ranlux3_01.html)
#define RAND_DET_RANLUX4_01				// Deterministic pseudo-random generator (http://www.boost.org/doc/libs/1_46_1/doc/html/boost/ranlux4_01.html)
#define RAND_DET_RANLUX64_3_01			// Deterministic pseudo-random generator (http://www.boost.org/doc/libs/1_46_1/doc/html/boost/ranlux64_3_01.html)
#define RAND_DET_RANLUX64_4_01			// Deterministic pseudo-random generator (http://www.boost.org/doc/libs/1_46_1/doc/html/boost/ranlux64_4_01.html)
#define RAND_NON_DET					// Non-deterministic random generator
*/

#if defined(RAND_DET_MINSTD_RAND0)
#include <boost/random/linear_congruential.hpp>
#elif defined(RAND_DET_MINSTD_RAND)
#include <boost/random/linear_congruential.hpp>
#elif defined(RAND_DET_RAND48)
#include <boost/random/linear_congruential.hpp>
#elif defined(RAND_DET_ECUYER1988)
#include <boost/random/additive_combine.hpp>
#elif defined(RAND_DET_KREUTZER1986)
#include <boost/random/shuffle_output.hpp>
#elif defined(RAND_DET_TAUS88)
#include <boost/random.hpp>
#elif defined(RAND_DET_HELLEKALEK1995)
#include <boost/random/inversive_congruential.hpp>
#elif defined(RAND_DET_MT11213B)
#include <boost/random/mersenne_twister.hpp>
#elif defined(RAND_DET_MT19937)
#include <boost/random/mersenne_twister.hpp>
#elif defined(RAND_DET_LAGGED_FIBONACCI607)
#include <boost/random/lagged_fibonacci.hpp>
#elif defined(RAND_DET_LAGGED_FIBONACCI1279)
#include <boost/random/lagged_fibonacci.hpp>
#elif defined(RAND_DET_LAGGED_FIBONACCI2281)
#include <boost/random/lagged_fibonacci.hpp>
#elif defined(RAND_DET_LAGGED_FIBONACCI3217)
#include <boost/random/lagged_fibonacci.hpp>
#elif defined(RAND_DET_LAGGED_FIBONACCI4423)
#include <boost/random/lagged_fibonacci.hpp>
#elif defined(RAND_DET_LAGGED_FIBONACCI9689)
#include <boost/random/lagged_fibonacci.hpp>
#elif defined(RAND_DET_LAGGED_FIBONACCI19937)
#include <boost/random/lagged_fibonacci.hpp>
#elif defined(RAND_DET_LAGGED_FIBONACCI23209)
#include <boost/random/lagged_fibonacci.hpp>
#elif defined(RAND_DET_LAGGED_FIBONACCI44497)
#include <boost/random/lagged_fibonacci.hpp>
#elif defined(RAND_DET_RANLUX3)
#include <boost/random/ranlux.hpp>
#elif defined(RAND_DET_RANLUX4)
#include <boost/random/ranlux.hpp>
#elif defined(RAND_DET_RANLUX64_3)
#include <boost/random/ranlux.hpp>
#elif defined(RAND_DET_RANLUX64_4)
#include <boost/random/ranlux.hpp>
#elif defined(RAND_DET_RANLUX3_01)
#include <boost/random/ranlux.hpp>
#elif defined(RAND_DET_RANLUX4_01)
#include <boost/random/ranlux.hpp>
#elif defined(RAND_DET_RANLUX64_3_01)
#include <boost/random/ranlux.hpp>
#elif defined(RAND_DET_RANLUX64_4_01)
#include <boost/random/ranlux.hpp>
#elif defined(RAND_NON_DET)
#include <boost/nondet_random.hpp>
#endif

#include <boost/random/random_number_generator.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/uniform_01.hpp>

using namespace boost;

namespace SimLib
{
	class Rand
	{
	public:
	#if defined(RAND_DET_MINSTD_RAND0)
		typedef minstd_rand0			UniformGenerator;
	#elif defined(RAND_DET_MINSTD_RAND)
		typedef minstd_rand				UniformGenerator;
	#elif defined(RAND_DET_RAND48)
		typedef rand48					UniformGenerator;
	#elif defined(RAND_DET_ECUYER1988)
		typedef ecuyer1988				UniformGenerator;
	#elif defined(RAND_DET_KREUTZER1986)
		typedef kreutzer1986			UniformGenerator;
	#elif defined(RAND_DET_TAUS88)
		typedef taus88					UniformGenerator;
	#elif defined(RAND_DET_HELLEKALEK1995)
		typedef hellekalek1995			UniformGenerator;
	#elif defined(RAND_DET_MT11213B)
		typedef mt11213b				UniformGenerator;
	#elif defined(RAND_DET_MT19937)
		typedef mt19937					UniformGenerator;
	#elif defined(RAND_DET_LAGGED_FIBONACCI607)
		typedef lagged_fibonacci607		UniformGenerator;
	#elif defined(RAND_DET_LAGGED_FIBONACCI1279)
		typedef lagged_fibonacci1279	UniformGenerator;
	#elif defined(RAND_DET_LAGGED_FIBONACCI2281)
		typedef lagged_fibonacci2281	UniformGenerator;
	#elif defined(RAND_DET_LAGGED_FIBONACCI3217)
		typedef lagged_fibonacci3217	UniformGenerator;
	#elif defined(RAND_DET_LAGGED_FIBONACCI4423)
		typedef lagged_fibonacci4423	UniformGenerator;
	#elif defined(RAND_DET_LAGGED_FIBONACCI9689)
		typedef lagged_fibonacci9689	UniformGenerator;
	#elif defined(RAND_DET_LAGGED_FIBONACCI19937)
		typedef lagged_fibonacci19937	UniformGenerator;
	#elif defined(RAND_DET_LAGGED_FIBONACCI23209)
		typedef lagged_fibonacci23209	UniformGenerator;
	#elif defined(RAND_DET_LAGGED_FIBONACCI44497)
		typedef lagged_fibonacci44497	UniformGenerator;
	#elif defined(RAND_DET_RANLUX3)
		typedef ranlux3					UniformGenerator;
	#elif defined(RAND_DET_RANLUX4)
		typedef ranlux4					UniformGenerator;
	#elif defined(RAND_DET_RANLUX64_3)
		typedef ranlux64_3				UniformGenerator;
	#elif defined(RAND_DET_RANLUX64_4)
		typedef ranlux64_4				UniformGenerator;
	#elif defined(RAND_DET_RANLUX3_01)
		typedef ranlux3_01				UniformGenerator;
	#elif defined(RAND_DET_RANLUX4_01)
		typedef ranlux4_01				UniformGenerator;
	#elif defined(RAND_DET_RANLUX64_3_01)
		typedef ranlux64_3_01			UniformGenerator;
	#elif defined(RAND_DET_RANLUX64_4_01)
		typedef ranlux64_4_01			UniformGenerator;
	#elif defined(RAND_NON_DET)
		typedef random_device			UniformGenerator;
	#endif
	
		typedef random_number_generator<UniformGenerator&, uint>
										UniformInteger;
		typedef variate_generator<UniformGenerator&, uniform_01<double> >
										Uniform01;

		static UniformGenerator			generator;

		static uniform_01<double>		dist01;

		static UniformInteger			uniformInteger;		// Returns a random integer in the interval [0, n)
		static Uniform01				uniform01;			// Returns a random real in the interval [0, 1)

		static void						Randomize();
	};
}