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

#include "SimHandler.h"
#include "SimTimerBase.h"
#include "EventTimer.h"
#include "ExceptionUnsupported.h"

namespace SimLib
{
	template<class T> class SimTimer : public SimTimerBase
	{
	protected:
		SimHandler&	sim;
		T&			object;
		void		(T::*handler)(SimTimerInfo* info);
		EventTimer*	evt;

	public:
		SimTimer(
			SimHandler&	sim,
			T&			object,
			void		(T::*handler)(SimTimerInfo* info) = NULL
			);

		virtual ~SimTimer();

		inline bool		IsSet() { return this->evt != NULL; }

		void			SetAt(__time time, SimTimerInfo* info = NULL);
		void			SetAt(__time time, void (T::*handler)(SimTimerInfo* info), SimTimerInfo* info = NULL);
		void			SetAfter(__time interval, SimTimerInfo* info = NULL);
		void			SetAfter(__time interval, void (T::*handler)(SimTimerInfo* info), SimTimerInfo* info = NULL);

		void			ResetAt(__time time);
		void			ResetAfter(__time interval);

		void			Cancel();

		virtual void	Finish(SimEvent* evt);
	};

	template<class T> SimTimer<T>::SimTimer(
		SimHandler&	sim,
		T&			object,
		void		(T::*handler)(SimTimerInfo* info)
		) : sim(sim), object(object), handler(handler), evt(NULL)
	{
	}

	template<class T> SimTimer<T>::~SimTimer()
	{
		// Cancel any pending events
		if(this->evt) this->sim.CancelEvent(this->evt->Time(), this->evt);
	}

	template<class T> void SimTimer<T>::SetAt(__time time, SimTimerInfo* info)
	{
		if(NULL != this->evt) throw ExceptionUnsupported(__FILE__, __LINE__, "SimTimer::SetAt - timer already set.");
		if(NULL == this->handler) throw ExceptionUnsupported(__FILE__, __LINE__, "SetTimer::SetAt - timer handler is null.");

		// Schedule next event
		EventTimer* evt = alloc EventTimer(*this, time, info);

		this->sim.ScheduleEventAt(time, evt);

		this->evt = evt;
	}

	template<class T> void SimTimer<T>::SetAt(__time time, void (T::*handler)(SimTimerInfo* info), SimTimerInfo* info)
	{
		if(NULL != this->evt) throw ExceptionUnsupported(__FILE__, __LINE__, "SimTimer::SetAt - timer already set.");

		// Set the handler
		this->handler = handler;

		// Schedule next event
		EventTimer* evt = alloc EventTimer(*this, time, info);

		this->sim.ScheduleEventAt(time, evt);

		this->evt = evt;
	}

	template<class T> void SimTimer<T>::SetAfter(__time interval, SimTimerInfo* info)
	{
		if(NULL != this->evt) throw ExceptionUnsupported(__FILE__, __LINE__, "SimTimer::SetAfter - timer already set.");
		if(NULL == this->handler) throw ExceptionUnsupported(__FILE__, __LINE__, "SetTimer::SetAfter - timer handler is null.");

		// Schedule next event
		EventTimer* evt = alloc EventTimer(*this, this->sim.Time() + interval, info);

		this->sim.ScheduleEventAt(this->sim.Time() + interval, evt);

		this->evt = evt;
	}

	template<class T> void SimTimer<T>::SetAfter(__time interval, void (T::*handler)(SimTimerInfo* info), SimTimerInfo* info)
	{
		if(NULL != this->evt) throw ExceptionUnsupported(__FILE__, __LINE__, "SimTimer::SetAfter - timer already set.");

		// Set the handler
		this->handler = handler;

		// Schedule next event
		EventTimer* evt = alloc EventTimer(*this, this->sim.Time() + interval, info);

		this->sim.ScheduleEventAt(this->sim.Time() + interval, evt);

		this->evt = evt;
	}

	template<class T> void SimTimer<T>::ResetAt(__time time)
	{
		if(NULL == this->evt) throw ExceptionUnsupported(__FILE__, __LINE__, "SimTimer::ResetAt - timer event is null.");
		if(NULL == this->handler) throw ExceptionUnsupported(__FILE__, __LINE__, "SetTimer::ResetAt - timer handler is null.");

		SimTimerInfo* info = this->evt->Info();

		// Cancel old event
		this->sim.CancelEvent(this->evt->Time(), this->evt);

		// Schedule next event
		EventTimer* evt = alloc EventTimer(*this, time, info);

		this->sim.ScheduleEventAt(time, evt);

		this->evt = evt;
	}

	template<class T> void SimTimer<T>::ResetAfter(__time interval)
	{
		if(NULL == this->evt) throw ExceptionUnsupported(__FILE__, __LINE__, "SimTimer::ResetAfter - timer event is null.");
		if(NULL == this->handler) throw ExceptionUnsupported(__FILE__, __LINE__, "SetTimer::ResetAfter - timer handler is null.");

		SimTimerInfo* info = this->evt->Info();

		// Cancel old event
		this->sim.CancelEvent(this->evt->Time(), this->evt);

		// Schedule next event
		EventTimer* evt = alloc EventTimer(*this, this->sim.Time() + interval, info);

		this->sim.ScheduleEventAt(this->sim.Time() + interval, evt);

		this->evt = evt;
	}

	template<class T> void SimTimer<T>::Cancel()
	{
		if(NULL == this->evt) throw ExceptionUnsupported(__FILE__, __LINE__, "SimTimer::Cancel - timer event is null.");

		// Cancel timer

		this->sim.CancelEvent(this->evt->Time(), this->evt);

		this->evt = NULL;
	}

	template<class T> void SimTimer<T>::Finish(SimEvent* evt)
	{
		// Copy the info
		SimTimerInfo* info = this->evt->Info();

		// Set event to null
		this->evt = NULL;

		// Call event handler
		(this->object.*this->handler)(info);
	}
}
