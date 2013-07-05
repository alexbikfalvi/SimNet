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
#include "SimEventList.h"

namespace SimLib
{
	SimEventList::SimEventList()
	{
		this->events = alloc std::multimap<__time, SimEvent*>();
	}

	SimEventList::SimEventList(SimModel* model)
	{
		this->events = alloc std::multimap<__time, SimEvent*>();

		if(NULL == model) return;

		__time time;
		for(unsigned int index = 0; index < model->Events(); index++)
		{
			try {
				SimEvent* evt = model->Event(index, time);
				this->Add(time, evt);
			} catch(...) { }
		}
	}

	SimEventList::~SimEventList()
	{
		for(std::multimap<__time, SimEvent*>::iterator iter = this->events->begin(); iter != this->events->end(); iter++)
			if(iter->second->AutoDelete())
				delete iter->second;
		delete this->events;
	}

	bool SimEventList::HasEvents()
	{
		return this->events->begin() != this->events->end();
	}

	SimEvent* SimEventList::GetEvent(__time &time)
	{
		if(this->events->begin() == this->events->end()) return NULL;

		std::multimap<__time, SimEvent*>::iterator iter = this->events->begin();
		SimEvent* evt = iter->second;
		time = iter->first;
		this->events->erase(iter);
		return evt;
	}

	unsigned int SimEventList::Size()
	{
		return this->events->size();
	}

	void SimEventList::Add(__time time, SimEvent *evt)
	{
		assert(evt);
		this->events->insert(std::pair<__time, SimEvent*>(time, evt));
	}

	void SimEventList::Cancel(__time time, SimEvent* evt)
	{
		assert(this->events->count(time));
	
		std::pair<std::multimap<__time, SimEvent*>::iterator, std::multimap<__time, SimEvent*>::iterator> bounds = 
			this->events->equal_range(time);

		for(std::multimap<__time, SimEvent*>::iterator iter = bounds.first; iter != bounds.second; iter++)
			if(iter->second == evt)
			{
				this->events->erase(iter);
				return;
			}
	}
}