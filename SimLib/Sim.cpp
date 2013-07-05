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
#include "Sim.h"

namespace SimLib
{
	Sim::Sim(
		SimModel*													model,
		IDelegate5<void, uint, __time, __time, time_t, time_t>*	progress
		)
	{
		assert(model);
		this->model = model;
		this->progress = progress;
		this->stop = 0;

		// Set the handler to the model
		this->model->SetHandler(this);

		// Create the event list
		this->events = alloc SimEventList(this->model);

		// Create the default progress delegate
		this->progressDefault = alloc Delegate5<Sim, void, uint, __time, __time, time_t, time_t>(
			*this,
			&Sim::Progress
			);
		if(NULL == this->progress) this->progress = this->progressDefault;

		// Simulation time
		this->time = 0;
	}

	Sim::~Sim()
	{
		delete this->events;
		delete this->progressDefault;
	}

	time_t Sim::Run()
	{
		// Real time
		time_t timeStart;
		time_t timeFinish;

		::time(&timeStart);

		// Initialize the model
		this->model->Initialize();

		while((this->events->HasEvents()) && (!this->stop))
		{
			// Get event from event list, execute and delete
			SimEvent* evt = this->events->GetEvent(time);
			assert(evt);

			evt->Process();

			// If the event is auto-deletable, delete the event
			if(evt->AutoDelete()) delete evt;
		}

		// Finalize the model
		this->model->Finalize();

		::time(&timeFinish);
		return timeFinish - timeStart;
	}

	time_t Sim::Run(__time maxTime, uint maxProgress)
	{
		// Real time
		time_t timeStart;
		time_t timeCurrent;
		time_t timeFinish;

		::time(&timeStart);

		// Progress
		uint progress = -1;

		// Initialize the model
		this->model->Initialize();

		while((this->events->HasEvents()) && (time <= maxTime))
		{
			// Get event from event list, execute and delete
			SimEvent* evt = this->events->GetEvent(time);
			assert(evt);

			evt->Process();

			// If the event is auto-deletable, delete the event
			if(evt->AutoDelete()) delete evt;

			// Check if progress has changed
			if((maxProgress * time / maxTime) != progress)
			{
				progress = (uint)(maxProgress * time / maxTime);

				// Calculate estimated remaining time
				::time(&timeCurrent);
				timeFinish = timeStart + (time_t)((time > 0) ? (timeCurrent - timeStart) * maxTime / time : 0);
			}
		}

		// Finalize the model
		this->model->Finalize();

		::time(&timeFinish);
		return timeFinish - timeStart;
	}

	__time Sim::Time()
	{
		return this->time;
	}

	void Sim::ScheduleEventNow(SimEvent* evt)
	{
		assert(evt);
		this->events->Add(this->time, evt);
	}

	void Sim::ScheduleEventAt(__time time, SimEvent* evt)
	{
		assert(evt);
		assert(time >= this->time);
		this->events->Add(time, evt);
	}

	void Sim::ScheduleEventAfter(__time delay, SimEvent* evt)
	{
		assert(evt);
		assert(this->time + delay >= this->time);
		this->events->Add(this->time + delay, evt);
	}

	void Sim::CancelEvent(__time time, SimEvent* evt)
	{
		assert(evt);
		if(time < this->time) return;	// If event is in the past, do nothing

		this->events->Cancel(time, evt);
		delete evt;
	}
}
