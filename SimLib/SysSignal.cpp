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
#include "SysSignal.h"
#include "ExceptionSignal.h"

namespace SimLib
{
	SysSignal::SysSignal()
	{
	#ifdef _MSC_VER
		// Initialize the event
		if(NULL == (this->evt = CreateEvent(
			NULL,	// default security attributes
			false,	// manual reset
			false,	// initial state (true is signaled)
			NULL	// event name
			))) throw ExceptionSignal(__FILE__, __LINE__, "create signal event failed");
	#elif __GNUC__
		// Initialize the mutex
		if(pthread_mutex_init(&this->mutex, NULL)) throw ExceptionSignal(__FILE__, __LINE__, "create signal mutex failed");
		// Initialize the condition
		if(pthread_cond_init(&this->cond, NULL)) throw ExceptionSignal(__FILE__, __LINE__, "create signal condition failed");
		// Lock the mutex
		if(pthread_mutex_lock(&this->mutex)) throw ExceptionSignal(__FILE__, __LINE__, "locking signal mutex failed");
	#endif
	}

	SysSignal::~SysSignal()
	{
	#ifdef _MSC_VER
		// Delete the event
		if(!CloseHandle(this->evt)) throw ExceptionSignal(__FILE__, __LINE__, "close signal event failed");
	#elif __GNUC__
		// Unlock the mutex
		if(pthread_mutex_unlock(&this->mutex)) throw ExceptionSignal(__FILE__, __LINE__, "unlocking signal mutex failed");
		// Delete the condition
		if(pthread_cond_destroy(&this->cond)) throw ExceptionSignal(__FILE__, __LINE__, "close signal condition failed");
		// Delete the mutex
		if(pthread_mutex_destroy(&this->mutex)) throw ExceptionSignal(__FILE__, __LINE__, "close signal mutex failed");
	#endif
	}

	void SysSignal::Raise()
	{
	#ifdef _MSC_VER
		// SysSignal the event
		if(!SetEvent(this->evt)) throw ExceptionSignal(__FILE__, __LINE__, "set signal to signaled state failed");
	#elif __GNUC__
		// Lock the mutex
		if(pthread_mutex_lock(&this->mutex)) throw ExceptionSignal(__FILE__, __LINE__, "locking signal mutex failed");
		// SysSignal the condition
		if(pthread_cond_signal(&this->cond)) throw ExceptionSignal(__FILE__, __LINE__, "set signal to signaled state failed");
		// Unlock the mutex
		if(pthread_mutex_unlock(&this->mutex)) throw ExceptionSignal(__FILE__, __LINE__, "unlocking signal mutex failed");
	#endif
	}

	void SysSignal::Wait()
	{
	#ifdef _MSC_VER
		if(WAIT_FAILED == WaitForSingleObject(this->evt, INFINITE)) throw ExceptionSignal(__FILE__, __LINE__, "waiting for signal failed");
	#elif __GNUC__
		if(pthread_cond_wait(&this->cond, &this->mutex)) throw ExceptionSignal(__FILE__, __LINE__, "waiting for signal failed");
	#endif
	}

	void SysSignal::Lock()
	{
		// Used only in __GNUC__ to lock the signal mutex
	#ifdef __GNUC__
		if(pthread_mutex_lock(&this->mutex)) throw ExceptionSignal(__FILE__, __LINE__, "locking signal mutex failed");
	#endif
	}

	void SysSignal::Unlock()
	{
		// Used only in __GNUC__ to unlock the signal mutex
	#ifdef __GNUC__
		if(pthread_mutex_unlock(&this->mutex)) throw ExceptionSignal(__FILE__, __LINE__, "unlocking signal mutex failed");
	#endif
	}
}
