/* 
 * Copyright (C) 2010-2012 Alex Bikfalvi
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

#include "SimEvent.h"
#include "SimTimerBase.h"
#include "SimTimerInfo.h"

namespace SimLib
{
	class EventTimer : public SimEvent
	{
	protected:
		SimTimerBase&	timer;
		__time			time;
		SimTimerInfo*	info;

	public:
		EventTimer(
			SimTimerBase&	timer,
			__time			time,
			SimTimerInfo*	info
			);
		virtual ~EventTimer() { }

		virtual void			Process();
		virtual void			Clean() { }
	
		inline __time			Time() { return this->time; }
		inline SimTimerInfo*	Info() { return this->info; }
	};
}
