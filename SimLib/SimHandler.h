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

namespace SimLib
{
	class SimHandler
	{
	public:
		SimHandler() { }
		virtual ~SimHandler() { }

		virtual __time	Time() = 0;
		virtual void	Stop() = 0;
		virtual void	ScheduleEventNow(SimEvent* evt) = 0;
		virtual void	ScheduleEventAt(__time time, SimEvent* evt) = 0;
		virtual void	ScheduleEventAfter(__time delay, SimEvent* evt) = 0;
		virtual void	CancelEvent(__time time, SimEvent* evt) = 0;
	};
}
