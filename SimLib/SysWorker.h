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

#include "SysWorkItem.h"

#ifdef _MSC_VER
#elif __GNUC__
#include <sched.h>
#include <errno.h>
#endif

namespace SimLib
{
	class SysWorker
	{
	public:
		enum EWorkerState
		{
			WORKER_STATE_STOPPED = 0,
			WORKER_STATE_STARTING = 1,
			WORKER_STATE_STARTED_IDLE = 2,
			WORKER_STATE_STARTED_BUSY = 3,
			WORKER_STATE_STOPPING = 4
		};

	private:
		unsigned int		id;
		EWorkerState		state;

		unsigned int		queueSize;
		unsigned int		queueCount;
		unsigned int		queuePtr;
		SysWorkItem**		queue;

	#ifdef _MSC_VER
		HANDLE				threadHandle;	// Thread handle
		DWORD				threadId;		// Thread id
		CRITICAL_SECTION	threadMutex;	// Thread mutex : used to synchronize shared variables with the worker thread

		HANDLE				stateHandle;	// State event handle
		HANDLE				workHandle;		// Work event handle
	#elif __GNUC__
		pthread_t			threadHandle;	// Thread handle
		pthread_mutex_t		threadMutex;	// Thread mutex

		pthread_cond_t		stateCond;		// State condition
		pthread_mutex_t		stateMutex;		// State mutex

		pthread_cond_t		workCond;		// Work condition
		pthread_mutex_t		workMutex;		// Work mutex
	#endif

	public:
		SysWorker(
			unsigned int	id,
			unsigned int	queueSize
			);
		~SysWorker();
	
		void				Start();
		void				Stop();

		void				Enqueue(SysWorkItem* item);

	private:

	#ifdef _MSC_VER
		static DWORD WINAPI Execute(__in LPVOID parameter);
	#elif __GNUC__
		static void*		Execute(void* parameter);
	#endif
	};
}
