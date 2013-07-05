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

#include "Headers.h"
#include "SimEvent.h"
#include "SimModel.h"

namespace SimLib
{
	class SimEventList
	{
	private:
		std::multimap<__time, SimEvent*>*	events;

	public:
		SimEventList();
		SimEventList(SimModel* model);
		~SimEventList();

		bool            HasEvents();
		SimEvent*		GetEvent(__time &time);
		unsigned int    Size();

		void            Add(__time time, SimEvent* evt);
		void            Cancel(__time time, SimEvent* evt);
	};
}
