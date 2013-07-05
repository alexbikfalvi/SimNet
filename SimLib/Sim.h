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

#include "SimHandler.h"
#include "SimModel.h"
#include "SimEventList.h"
#include "Delegate.h"

namespace SimLib
{
	class Sim : public SimHandler
	{
	private:
		SimModel*		model;
		SimEventList*	events;
		__time			time;
		IDelegate5<
			void,
			uint,
			__time,
			__time,
			time_t,
			time_t>*	progress;
		Delegate5<
			Sim,
			void,
			uint,
			__time,
			__time,
			time_t,
			time_t>*	progressDefault;

		bool			stop;

	public:
		Sim(
			SimModel*													model,
			IDelegate5<void, uint, __time, __time, time_t, time_t>*	progress = NULL
			);
		virtual ~Sim();

		time_t			Run();
		time_t			Run(__time maxTime, uint maxProgress = 100);

		virtual __time	Time();
		virtual void	Stop() { this->stop = true; }
		virtual void	ScheduleEventNow(SimEvent* evt);
		virtual void	ScheduleEventAt(__time time, SimEvent* evt);
		virtual void	ScheduleEventAfter(__time delay, SimEvent* evt);
		virtual void	CancelEvent(__time time, SimEvent* evt);

	private:
		void			Progress(uint, __time, __time, time_t, time_t) { }
	};
}
