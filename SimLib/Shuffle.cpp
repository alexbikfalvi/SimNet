/* 
 * Copyright (C) 2011 Alex Bikfalvi
 * Copyright (C) 2004 Ben Pfaff ( http://benpfaff.org/writings/clc/shuffle.html )
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
#include "Shuffle.h"
#include "Rand.h"

namespace SimLib
{
	Shuffle::Shuffle(
		uint	count
		)
	{
		assert(count);

		this->count = count;

		this->dirMapping = alloc uint[this->count];
		this->invMapping = alloc uint[this->count];

		for(uint index = 0; index < this->count; index++)
			this->dirMapping[index] = index;

		this->Randomize();
	}

	Shuffle::~Shuffle()
	{
		delete[] this->dirMapping;
		delete[] this->invMapping;
	}

	void Shuffle::Randomize()
	{
		uint i2;
		uint tmp;
	
		for(uint i1 = 0; i1 < this->count - 1; i1++)
		{
			i2 = i1 + Rand::uniformInteger(this->count - i1);
			assert(i2 < this->count);

			tmp = this->dirMapping[i2];
			this->dirMapping[i2] = this->dirMapping[i1];
			this->dirMapping[i1] = tmp;
		}

		for(uint index = 0; index < count; index++)
			this->invMapping[this->dirMapping[index]] = index;
	}
}