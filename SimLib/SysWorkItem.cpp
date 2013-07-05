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
#include "SysWorkItem.h"
#include "ExceptionSignal.h"
#include "ExceptionMutex.h"

namespace SimLib
{
	SysWorkItem::SysWorkItem()
	{
		// Set initial work item state
		this->state = PENDING;

		// Create work item signal
	#ifdef _MSC_VER
		// Initialize the work item event
		if(NULL == (this->evt = CreateEvent(
			NULL,	// default security attributes
			false,	// manual reset
			false,	// initial state (true is signaled)
			NULL	// event name
			))) throw ExceptionSignal(__FILE__, __LINE__, "create work item signal failed");
	#elif __GNUC__
		// Initialize the work item mutex
		if(pthread_mutex_init(&this->mutex, NULL)) throw ExceptionMutex(__FILE__, __LINE__, "create work item mutex failed");
		// Initialize the work item condition
		if(pthread_cond_init(&this->cond, NULL)) throw ExceptionSignal(__FILE__, __LINE__, "create work item signal failed");
		// Lock the mutex
		if(pthread_mutex_lock(&this->mutex)) throw ExceptionSignal(__FILE__, __LINE__, "locking work item mutex failed");
	#endif
	}

	SysWorkItem::~SysWorkItem()
	{
	#ifdef _MSC_VER
		// Delete the work item event
		if(!CloseHandle(this->evt)) throw ExceptionSignal(__FILE__, __LINE__, "close work item signal failed");
	#elif __GNUC__
		// Unlock the mutex
		if(pthread_mutex_unlock(&this->mutex)) throw ExceptionSignal(__FILE__, __LINE__, "unlocking work item mutex failed");
		// Delete the work item condition
		if(pthread_cond_destroy(&this->cond)) throw ExceptionSignal(__FILE__, __LINE__, "close work item signal failed");
		// Delete the work item mutex
		if(pthread_mutex_destroy(&this->mutex)) throw ExceptionMutex(__FILE__, __LINE__, "close work item mutex failed");
	#endif
	}

	void SysWorkItem::SysSignal(WorkItemState state)
	{
		this->state = state;

		// SysSignal the completion of the work item
	#ifdef _MSC_VER
		if(!SetEvent(this->evt)) throw ExceptionSignal(__FILE__, __LINE__, "set work item signal to signaled state failed");
	#elif __GNUC__
		// Lock the mutex
		if(pthread_mutex_lock(&this->mutex)) throw ExceptionSignal(__FILE__, __LINE__, "locking work item mutex failed");
		// SysSignal the condition
		if(pthread_cond_signal(&this->cond)) throw ExceptionSignal(__FILE__, __LINE__, "set work item signal to signaled state failed");
		// Unlock the mutex
		if(pthread_mutex_unlock(&this->mutex)) throw ExceptionSignal(__FILE__, __LINE__, "unlocking work item mutex failed");
	#endif
	}

	void SysWorkItem::Wait()
	{
	#ifdef _MSC_VER
		if(WAIT_FAILED == WaitForSingleObject(this->evt, INFINITE)) throw ExceptionSignal(__FILE__, __LINE__, "waiting for work item signal failed");
	#elif __GNUC__
		if(pthread_cond_wait(&this->cond, &this->mutex)) throw ExceptionSignal(__FILE__, __LINE__, "waiting for work item signal failed");
	#endif
	}
}
