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
#include "SysWorker.h"
#include "ExceptionWorker.h"
#include "ExceptionThread.h"
#include "ExceptionSignal.h"
#include "ExceptionMutex.h"

namespace SimLib
{
	SysWorker::SysWorker(
		unsigned int	id,
		unsigned int	queueSize
		)
	{
		this->id = id;
		this->queueSize = queueSize;
		this->queueCount = 0;
		this->queuePtr = 0;

		// Create queue
		this->queue = new SysWorkItem*[this->queueSize];

		// Initial state
		this->state = WORKER_STATE_STOPPED;

	#ifdef _MSC_VER
		// Initialize the thread mutex
		InitializeCriticalSection(&this->threadMutex);

		// Initialize the state signal
		if(NULL == (this->stateHandle = CreateEvent(
			NULL,	// default security attributes
			false,	// manual reset
			false,	// initial state (true is signaled)
			NULL	// event name
			))) throw ExceptionSignal(__FILE__, __LINE__, "create state signal failed");

		// Initialize the work signal
		if(NULL == (this->workHandle = CreateEvent(
			NULL,	// default security attributes
			false,	// manual reset
			false,	// initial state (true is signaled)
			NULL	// event name
			))) throw ExceptionSignal(__FILE__, __LINE__, "create work signal failed");

	#elif __GNUC__
		// Initialize the thread mutex
		if(pthread_mutex_init(&this->threadMutex, NULL)) throw ExceptionMutex(__FILE__, __LINE__, "create thread mutex failed");
	
		// Initialize the state mutex
		if(pthread_mutex_init(&this->stateMutex, NULL)) throw ExceptionMutex(__FILE__, __LINE__, "create state mutex failed");
		// Initialize the state codition
		if(pthread_cond_init(&this->stateCond, NULL)) throw ExceptionSignal(__FILE__, __LINE__, "create state signal failed");

		// Initialize the work mutex
		if(pthread_mutex_init(&this->workMutex, NULL)) throw ExceptionMutex(__FILE__, __LINE__, "create work mutex failed");
		// Initialize the work codition
		if(pthread_cond_init(&this->workCond, NULL)) throw ExceptionSignal(__FILE__, __LINE__, "create work signal failed");
	#endif
	}

	SysWorker::~SysWorker()
	{
	#ifdef _MSC_VER
		// Close the thread mutex
		DeleteCriticalSection(&this->threadMutex);

		// Close the state signal
		if(!CloseHandle(this->stateHandle)) throw ExceptionSignal(__FILE__, __LINE__, "close state signal failed");

		// Close the work signal
		if(!CloseHandle(this->workHandle)) throw ExceptionSignal(__FILE__, __LINE__, "close work signal failed");

	#elif __GNUC__
		// Close the thread mutex
		if(pthread_mutex_destroy(&this->threadMutex)) throw ExceptionMutex(__FILE__, __LINE__, "close thread mutex failed");

		// Close the state condition
		if(pthread_cond_destroy(&this->stateCond)) throw ExceptionSignal(__FILE__, __LINE__, "close state signal failed");
		// Close the state mutex
		if(pthread_mutex_destroy(&this->stateMutex)) throw ExceptionMutex(__FILE__, __LINE__, "close state mutex failed");

		// Close the work condition
		if(pthread_cond_destroy(&this->workCond)) throw ExceptionSignal(__FILE__, __LINE__, "close work signal failed");
		// Close the work mutex
		if(pthread_mutex_destroy(&this->workMutex)) throw ExceptionMutex(__FILE__, __LINE__, "close work mutex failed");
	#endif

		// Delete queue
		delete[] this->queue;
	}

	void SysWorker::Start()
	{
	#ifdef _MSC_VER
		// Lock the thread mutex
		EnterCriticalSection(&this->threadMutex);
		try
		{
			// Check the worker state
			if(WORKER_STATE_STOPPED != this->state) throw ExceptionWorker(__FILE__, __LINE__, this->id, "worker is not stopped");

			// Change the worker state
			this->state = WORKER_STATE_STARTING;

			// Start thread
			if(!(this->threadHandle = CreateThread(
				NULL,					// default security attributes
				0,						// default stack size
				&SysWorker::Execute,	// thread function address
				this,					// parameter
				0,						// creation flags
				&this->threadId			// thread ID
				))) throw ExceptionThread(__FILE__, __LINE__, this->threadId, "create thread failed");
		}
		catch(...)
		{
			// Unlock the thread mutex and re-throw exception
			LeaveCriticalSection(&this->threadMutex);
			throw;
		}
		// Unlock the thread mutex
		LeaveCriticalSection(&this->threadMutex);

		// Wait for thread to signal the state
		if(WAIT_FAILED == WaitForSingleObject(this->stateHandle, INFINITE)) throw ExceptionSignal(__FILE__, __LINE__, "waiting for worker start signal failed");
	#elif __GNUC__
		// Lock the thread mutex
		if(pthread_mutex_lock(&this->threadMutex)) throw ExceptionMutex(__FILE__, __LINE__, "locking thread mutex failed");
		// Lock the state mutex
		if(pthread_mutex_lock(&this->stateMutex)) throw ExceptionMutex(__FILE__, __LINE__, "locking state mutex failed");
		try
		{
			// Check the worker state
			if(WORKER_STATE_STOPPED != this->state) throw ExceptionWorker(__FILE__, __LINE__, this->id, "worker is not stopped");

			// Change the worker state
			this->state = WORKER_STATE_STARTING;

			if(pthread_create(
				&this->threadHandle,	// thread handle
				NULL,					// default thread attribute
				&SysWorker::Execute,	// thread function address
				(void*)this				// parameter
				)) throw ExceptionThread(__FILE__, __LINE__, this->threadHandle, "create thread failed");

	#ifdef WORKER_PRIORITY
					struct sched_param schedParam;
					int schedPolicy;

					pthread_getschedparam(this->threadHandle, &schedPolicy, &schedParam);

					int schedPriorityMax = sched_get_priority_max(schedPolicy);
					int schedPriorityMin = sched_get_priority_min(schedPolicy);

					cout << endl;
					cout << "Thread policy is: " << schedPolicy << endl;
					cout << "Thread priority is: " << schedParam.__sched_priority << endl;
					cout << "Scheduling policy priority range is: " << schedPriorityMin << ".." << schedPriorityMax << endl;
					cout << "Changing thread priority to real-time: ";

					schedParam.__sched_priority = 1;
					switch(pthread_setschedparam(this->threadHandle, SCHED_FIFO, &schedParam))
					{
						case EINVAL: cout << "invalid policy or parameter." << endl; break;
						case ENOTSUP: cout << "not supported." << endl; break;
						case EPERM: cout << "no permissions." << endl; break;
						case ESRCH: cout << "thread does not exist." << endl; break;
						default: cout << "success." << endl;
					}
	#endif
		}
		catch(...)
		{
			// Unlock the thread mutex and re-throw exception
			if(pthread_mutex_unlock(&this->threadMutex)) throw ExceptionMutex(__FILE__, __LINE__, "unlocking thread mutex failed");
			throw;
		}
		// Unlock the thread mutex
		if(pthread_mutex_unlock(&this->threadMutex)) throw ExceptionMutex(__FILE__, __LINE__, "unlocking thread mutex failed");

		// Wait for thread to signal the state
		if(pthread_cond_wait(&this->stateCond, &this->stateMutex)) throw ExceptionSignal(__FILE__, __LINE__, "waiting for worker start signal failed");
	#endif
	}

	void SysWorker::Stop()
	{
	#ifdef _MSC_VER
		// Lock the thread mutex
		EnterCriticalSection(&this->threadMutex);
		try
		{
			// Check the worker state
			if((WORKER_STATE_STARTED_IDLE != this->state) && (WORKER_STATE_STARTED_BUSY != this->state))
				throw ExceptionWorker(__FILE__, __LINE__, this->id, "worker is not started");

			// Save the old worker state
			EWorkerState oldState = this->state;

			// Change the worker state
			this->state = WORKER_STATE_STOPPING;

			// Send work change signal
			if(!SetEvent(this->workHandle)) throw ExceptionSignal(__FILE__, __LINE__, "set work signal to signaled state failed");
		}
		catch(...)
		{
			// Unlock the thread mutex and re-throw exception
			LeaveCriticalSection(&this->threadMutex);
			throw;
		}
		// Unlock the thread mutex
		LeaveCriticalSection(&this->threadMutex);

		// Wait for thread to complete
		if(WAIT_FAILED == WaitForSingleObject(this->threadHandle, INFINITE)) throw ExceptionSignal(__FILE__, __LINE__, "waiting for worker stop signal failed");

		// Close the thread handle
		if(!CloseHandle(this->threadHandle)) throw ExceptionThread(__FILE__, __LINE__, this->threadId, "close thread failed");

	#elif __GNUC__
		// Lock the thread mutex
		if(pthread_mutex_lock(&this->threadMutex)) throw ExceptionMutex(__FILE__, __LINE__, "locking thread mutex failed");
		try
		{
			// Check the worker state
			if((WORKER_STATE_STARTED_IDLE != this->state) && (WORKER_STATE_STARTED_BUSY != this->state))
				throw ExceptionWorker(__FILE__, __LINE__, this->id, "worker is not started");

			// Save the old worker state
			EWorkerState oldState = this->state;

			// Change the worker state
			this->state = WORKER_STATE_STOPPING;

			// If can acquire a lock on the work mutex
			if(!pthread_mutex_trylock(&this->workMutex))
			{
				try
				{
					// Send work signal
					if(pthread_cond_signal(&this->workCond)) throw ExceptionSignal(__FILE__, __LINE__, "set work signal to signaled state failed");
				}
				catch(...)
				{
					// Unlock the work mutex and re-throw the exception
					if(pthread_mutex_unlock(&this->workMutex)) throw ExceptionMutex(__FILE__, __LINE__, "unlocking work mutex failed");
					throw;
				}
				// Unlock the work mutex
				if(pthread_mutex_unlock(&this->workMutex)) throw ExceptionMutex(__FILE__, __LINE__, "unlocking work mutex failed");
			}
		}
		catch(...)
		{
			// Unlock the thread mutex and re-throw exception
			if(pthread_mutex_unlock(&this->threadMutex)) throw ExceptionMutex(__FILE__, __LINE__, "unlocking thread mutex failed");
			throw;
		}
		// Unlock the thread mutex
		if(pthread_mutex_unlock(&this->threadMutex)) throw ExceptionMutex(__FILE__, __LINE__, "unlocking thread mutex failed");

		// Wait for thread to complete
		if(pthread_join(this->threadHandle, NULL)) throw ExceptionSignal(__FILE__, __LINE__, "waiting for worker stop signal failed");
		// Unlock the state mutex
		if(pthread_mutex_unlock(&this->stateMutex)) throw ExceptionMutex(__FILE__, __LINE__, "unlocking the state mutex failed");
	#endif
	}

	void SysWorker::Enqueue(SysWorkItem* item)
	{
	#ifdef _MSC_VER
		// Synchronize access to shared variables
		EnterCriticalSection(&this->threadMutex);
		try
		{
			// Check the worker state
			if((WORKER_STATE_STARTED_IDLE != this->state) && (WORKER_STATE_STARTED_BUSY != this->state)) throw ExceptionWorker(__FILE__, __LINE__, this->id, "cannot enqueue item; worker to started");

			// Check the worker queue is not empty
			if(this->queueCount >= this->queueSize) throw ExceptionWorker(__FILE__, __LINE__, this->id, "worker queue is full");

			// Add the work item to the queue
			this->queue[(this->queuePtr + this->queueCount) % this->queueSize] = item;
			this->queueCount++;

			// If the worker is in idle, send work change signal
			if(WORKER_STATE_STARTED_IDLE == this->state)
				if(!SetEvent(this->workHandle)) throw ExceptionSignal(__FILE__, __LINE__, "set work signal to signaled state failed");
		}
		catch(...)
		{
			LeaveCriticalSection(&this->threadMutex);
			throw;
		}
		LeaveCriticalSection(&this->threadMutex);
	#elif __GNUC__
		// Synchronize access to shared variables
		if(pthread_mutex_lock(&this->threadMutex)) throw ExceptionMutex(__FILE__, __LINE__, "locking thread mutex failed");
		try
		{
			// Check the worker state
			if((WORKER_STATE_STARTED_IDLE != this->state) && (WORKER_STATE_STARTED_BUSY != this->state)) throw ExceptionWorker(__FILE__, __LINE__, this->id, "cannot enqueue item; worker to started");

			// Check the worker queue is not empty
			if(this->queueCount >= this->queueSize) throw ExceptionWorker(__FILE__, __LINE__, this->id, "worker queue is full");

			// Add the work item to the queue
			this->queue[(this->queuePtr + this->queueCount) % this->queueSize] = item;
			this->queueCount++;

			// If can acquire a lock on the work mutex
			if(!pthread_mutex_trylock(&this->workMutex))
			{
				try
				{
					// Send work signal
					if(pthread_cond_signal(&this->workCond)) throw ExceptionSignal(__FILE__, __LINE__, "set work signal to signaled state failed");
				}
				catch(...)
				{
					// Unlock the work mutex and re-throw the exception
					if(pthread_mutex_unlock(&this->workMutex)) throw ExceptionMutex(__FILE__, __LINE__, "unlocking work mutex failed");
					throw;
				}
				// Unlock the work mutex
				if(pthread_mutex_unlock(&this->workMutex)) throw ExceptionMutex(__FILE__, __LINE__, "unlocking work mutex failed");
			}
		}
		catch(...)
		{
			if(pthread_mutex_unlock(&this->threadMutex)) throw ExceptionMutex(__FILE__, __LINE__, "unlocking thread mutex failed");
			throw;
		}
		if(pthread_mutex_unlock(&this->threadMutex)) throw ExceptionMutex(__FILE__, __LINE__, "unlocking thread mutex failed");
	#endif
	}

	#ifdef _MSC_VER
	DWORD SysWorker::Execute(__in LPVOID parameter)
	{
		// Get the object
		SysWorker* worker = (SysWorker*)(parameter);

		// Synchronize access to shared variables
		EnterCriticalSection(&worker->threadMutex);
		try
		{
			// Change the state
			worker->state = WORKER_STATE_STARTED_IDLE;

			// Send state change signal
			if(!SetEvent(worker->stateHandle)) throw ExceptionSignal(__FILE__, __LINE__, "set state signal to signaled state failed");
		}
		catch(...)
		{
			LeaveCriticalSection(&worker->threadMutex);
			throw;
		}
		LeaveCriticalSection(&worker->threadMutex);

		// SysWorker execution loop
		do
		{
			// Wait for a work signal
			if(WAIT_FAILED == WaitForSingleObject(worker->workHandle, INFINITE)) throw ExceptionSignal(__FILE__, __LINE__, "waiting for work signal failed");

			// Local copy of the work item
			SysWorkItem* item = NULL;

			do
			{
				// Synchronize access to shared variables
				EnterCriticalSection(&worker->threadMutex);
				try
				{
					// If the worker is not stopping and the worker queue is not empty
					if(WORKER_STATE_STOPPING != worker->state)
					{
						if(worker->queueCount)
						{
							// Pop the oldest work item from the queue
							item = worker->queue[worker->queuePtr];

							// Shift the end of the queue
							worker->queuePtr = (++worker->queuePtr) % worker->queueSize;
							worker->queueCount--;

							// Set worker state to busy
							worker->state = WORKER_STATE_STARTED_BUSY;
						}
						else
						{
							// Set item to null
							item = NULL;

							// Set worker state to idle
							worker->state = WORKER_STATE_STARTED_IDLE;
						}
					}
				}
				catch(...)
				{
					LeaveCriticalSection(&worker->threadMutex);
					throw;
				}
				LeaveCriticalSection(&worker->threadMutex);

				// If the work item is not null
				if(item && (WORKER_STATE_STOPPING != worker->state))
				{
					try
					{
						// Execute the work item
						item->Execute();

						// SysSignal work item completion : success
						item->SysSignal(SysWorkItem::COMPLETED_SUCCESS);
					}
					catch(...)
					{
						// SysSignal work item completion : fail
						item->SysSignal(SysWorkItem::COMPLETED_FAIL);
					}
				}
			}
			while(item && (WORKER_STATE_STOPPING != worker->state));

		}
		while(WORKER_STATE_STOPPING != worker->state);

		// Synchronize access to shared variables
		EnterCriticalSection(&worker->threadMutex);
		try
		{
			// Change worker state
			worker->state = WORKER_STATE_STOPPED;

			// SysSignal all work items remaining in the queue
			for(; worker->queueCount; worker->queueCount--)
			{
				// Pop item from the queue
				SysWorkItem* item = worker->queue[worker->queuePtr];

				// Shift the end of the queue
				worker->queuePtr = (++worker->queuePtr) % worker->queueSize;

				// SysSignal work item completion : pending
				item->SysSignal(SysWorkItem::PENDING);
			}
		}
		catch(...)
		{
			LeaveCriticalSection(&worker->threadMutex);
			throw;
		}
		LeaveCriticalSection(&worker->threadMutex);

		return 0;
	}
	#elif __GNUC__
	void* SysWorker::Execute(void* parameter)
	{
		// Get the object
		SysWorker* worker = (SysWorker*)(parameter);

		// Lock the work mutex
		if(pthread_mutex_lock(&worker->workMutex)) throw ExceptionMutex(__FILE__, __LINE__, "locking work mutex failed");

		try
		{
			// Synchronize access to shared variables
			if(pthread_mutex_lock(&worker->threadMutex)) throw ExceptionMutex(__FILE__, __LINE__, "locking thread mutex failed");
			try
			{
				// Change the state
				worker->state = WORKER_STATE_STARTED_IDLE;

				// Send state change signal

				// Lock the state mutex
				if(pthread_mutex_lock(&worker->stateMutex)) throw ExceptionMutex(__FILE__, __LINE__, "locking state mutex failed");
				try
				{
					// Send signal
					if(pthread_cond_signal(&worker->stateCond)) throw ExceptionSignal(__FILE__, __LINE__, "set state signal to signaled state failed");
				}
				catch(...)
				{
					// Unlock the state mutex
					if(pthread_mutex_unlock(&worker->stateMutex)) throw ExceptionMutex(__FILE__, __LINE__, "unlocking state mutex failed");
					throw;
				}
				if(pthread_mutex_unlock(&worker->stateMutex)) throw ExceptionMutex(__FILE__, __LINE__, "unlocking state mutex failed");
			}
			catch(...)
			{
				if(pthread_mutex_unlock(&worker->threadMutex)) throw ExceptionMutex(__FILE__, __LINE__, "unlocking thread mutex failed");
				throw;
			}
			if(pthread_mutex_unlock(&worker->threadMutex)) throw ExceptionMutex(__FILE__, __LINE__, "unlocking thread mutex failed");

			// SysWorker execution loop
			do
			{
				// Wait for a work signal
				if(pthread_cond_wait(&worker->workCond, &worker->workMutex)) throw ExceptionSignal(__FILE__, __LINE__, "waiting for work signal failed");

				// Local copy of the work item
				SysWorkItem* item = NULL;

				do
				{
					// Synchronize access to shared variables
					if(pthread_mutex_lock(&worker->threadMutex)) throw ExceptionMutex(__FILE__, __LINE__, "locking thread mutex failed");
					try
					{
						// If the worker is not stopping and the worker queue is not empty
						if(WORKER_STATE_STOPPING != worker->state)
						{
							if(worker->queueCount)
							{
								// Pop the oldest work item from the queue
								item = worker->queue[worker->queuePtr];

								// Shift the end of the queue
								worker->queuePtr = (++worker->queuePtr) % worker->queueSize;
								worker->queueCount--;

								// Set worker state to busy
								worker->state = WORKER_STATE_STARTED_BUSY;
							}
							else
							{
								// Set item to null
								item = NULL;

								// Set worker state to idle
								worker->state = WORKER_STATE_STARTED_IDLE;
							}
						}
					}
					catch(...)
					{
						if(pthread_mutex_unlock(&worker->threadMutex)) throw ExceptionMutex(__FILE__, __LINE__, "unlocking thread mutex failed");
						throw;
					}
					if(pthread_mutex_unlock(&worker->threadMutex)) throw ExceptionMutex(__FILE__, __LINE__, "unlocking thread mutex failed");

					// If the work item is not null
					if(item && (WORKER_STATE_STOPPING != worker->state))
					{
						try
						{
							// Execute the work item
							item->Execute();

							// SysSignal work item completion : success
							item->SysSignal(SysWorkItem::COMPLETED_SUCCESS);
						}
						catch(...)
						{
							// SysSignal work item completion : fail
							item->SysSignal(SysWorkItem::COMPLETED_FAIL);
						}
					}
				}
				while(item && (WORKER_STATE_STOPPING != worker->state));

			}
			while(WORKER_STATE_STOPPING != worker->state);

			// Synchronize access to shared variables
			if(pthread_mutex_lock(&worker->threadMutex)) throw ExceptionMutex(__FILE__, __LINE__, "locking thread mutex failed");
			try
			{
				// Change worker state
				worker->state = WORKER_STATE_STOPPED;

				// SysSignal all work items remaining in the queue
				for(; worker->queueCount; worker->queueCount--)
				{
					// Pop item from the queue
					SysWorkItem* item = worker->queue[worker->queuePtr];

					// Shift the end of the queue
					worker->queuePtr = (++worker->queuePtr) % worker->queueSize;

					// SysSignal work item completion : pending
					item->SysSignal(SysWorkItem::PENDING);
				}
			}
			catch(...)
			{
				if(pthread_mutex_unlock(&worker->threadMutex)) throw ExceptionMutex(__FILE__, __LINE__, "unlocking thread mutex failed");
				throw;
			}
			if(pthread_mutex_unlock(&worker->threadMutex)) throw ExceptionMutex(__FILE__, __LINE__, "unlocking thread mutex failed");
		}
		catch(...)
		{
			// Unlock the work mutex and re-throw exception
			if(pthread_mutex_unlock(&worker->workMutex)) throw ExceptionMutex(__FILE__, __LINE__, "unlocking work mutex failed");
			throw;
		}
		// Unlock the work mutex
		if(pthread_mutex_unlock(&worker->workMutex)) throw ExceptionMutex(__FILE__, __LINE__, "unlocking work mutex failed");

		return 0;
	}
	#endif
}