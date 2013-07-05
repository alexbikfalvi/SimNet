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

#ifdef _MSC_VER
#include <windows.h>
#elif __GNUC__
#include <pthread.h>
#endif

namespace SimLib
{
	class SysSignal
	{
	private:
	#ifdef _MSC_VER
		HANDLE			evt;
	#elif __GNUC__
		pthread_mutex_t	mutex;
		pthread_cond_t	cond;
	#endif

	public:
		SysSignal();
		virtual ~SysSignal();

		void	Raise();
		void	Wait();
		void	Lock();
		void	Unlock();
	};
}
