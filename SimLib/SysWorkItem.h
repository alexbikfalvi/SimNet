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
	class SysWorkItem
	{
	public:
		enum WorkItemState
		{
			PENDING = 0,
			COMPLETED_SUCCESS = 1,
			COMPLETED_FAIL = 2
		};

	protected:
		WorkItemState	state;

	#ifdef _MSC_VER
		HANDLE			evt;
	#elif __GNUC__
		pthread_mutex_t	mutex;
		pthread_cond_t	cond;
	#endif

	public:
		SysWorkItem();
		virtual ~SysWorkItem();

		void					SysSignal(WorkItemState state);
		void					Wait();
		inline WorkItemState	State() { return this->state; }

		virtual void			Execute() = 0;
	};

	template<typename T, typename I, typename O> class WorkItemImpl : public SysWorkItem
	{
	private:
		T*		object;
		void	(T::*func)(I, O);
		I		in;
		O		out;

	public:
		WorkItemImpl(T* object, void (T::*func)(I, O))
		{
			this->object = object;
			this->func = func;
		}
		WorkItemImpl(T* object, void (T::*func)(I, O), I in, O out)
		{
			this->object = object;
			this->func = func;
			this->in = in;
			this->out = out;
		}
		virtual ~WorkItemImpl() { }

		void			Set(I in, O out)
		{
			// Reset initial work item state
			this->state = PENDING;

			// Set the parameters
			this->in = in;
			this->out = out;
		}

		virtual void	Execute()
		{
			(this->object->*this->func)(this->in, this->out);
		}
	};
}
