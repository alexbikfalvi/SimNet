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
#include "SysMutex.h"
#include "ExceptionMutex.h"

namespace SimLib
{
	SysMutex::SysMutex()
	{
	#ifdef _MSC_VER
		InitializeCriticalSection(&this->mutex);
	#elif __GNUC__
		if(pthread_mutex_init(&this->mutex, NULL)) throw ExceptionMutex(__FILE__, __LINE__, "initialize mutex failed");
	#endif
	}

	SysMutex::~SysMutex()
	{
	#ifdef _MSC_VER
		DeleteCriticalSection(&this->mutex);
	#elif __GNUC__
		if(pthread_mutex_destroy(&this->mutex)) throw ExceptionMutex(__FILE__, __LINE__, "destroy mutex failed");
	#endif
	}

	void SysMutex::Lock()
	{
	#ifdef _MSC_VER
		EnterCriticalSection(&this->mutex);
	#elif __GNUC__
		if(pthread_mutex_lock(&this->mutex)) throw ExceptionMutex(__FILE__, __LINE__, "lock mutex failed");
	#endif
	}

	void SysMutex::Unlock()
	{
	#ifdef _MSC_VER
		LeaveCriticalSection(&this->mutex);
	#elif __GNUC__
		if(pthread_mutex_unlock(&this->mutex)) throw ExceptionMutex(__FILE__, __LINE__, "unlock mutex failed");
	#endif
	}

	bool SysMutex::TryLock()
	{
	#ifdef _MSC_VER
		return TryEnterCriticalSection(&this->mutex) != 0;
	#elif __GNUC__
		return pthread_mutex_trylock(&this->mutex) == 0;
	#endif
	}
}
