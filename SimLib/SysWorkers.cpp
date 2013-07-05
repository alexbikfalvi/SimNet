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
#include "SysWorkers.h"

namespace SimLib
{
	SysWorkers::SysWorkers(
		unsigned int	workersCount,
		unsigned int	workersQueueSize
		)
	{
		this->workersCount = workersCount;
		this->workerLast = 0;

		// Create workers
		this->workers = new SysWorker*[this->workersCount];
		for(unsigned int index = 0; index < this->workersCount; index++)
			this->workers[index] = new SysWorker(index, workersQueueSize);
	}

	SysWorkers::~SysWorkers()
	{
		for(unsigned int index = 0; index < this->workersCount; index++)
			delete this->workers[index];
		delete[] this->workers;
	}

	void SysWorkers::Start()
	{
		std::cout << "\nStarting simulator worker threads : ";
		// Start all workers
		for(unsigned int index = 0; index < this->workersCount; index++)
		{
			// Start worker
			this->workers[index]->Start();
			std::cout << index << " ";
		}
		std::cout << "done!" << std::endl;

	#ifdef WORKER_PRIORITY
			std::cout << "Scheduling policies:" << std::endl;
			std::cout << "\tSCHED_OTHER min=" << sched_get_priority_min(SCHED_OTHER) << " max=" << sched_get_priority_max(SCHED_OTHER) << std::endl;
			std::cout << "\tSCHED_FIFO min=" << sched_get_priority_min(SCHED_FIFO) << " max=" << sched_get_priority_max(SCHED_FIFO) << std::endl;
			std::cout << "\tSCHED_RR min=" << sched_get_priority_min(SCHED_RR) << " max=" << sched_get_priority_max(SCHED_RR) << std::endl;
	#endif
	}

	void SysWorkers::Stop()
	{
		std::cout << "\nStopping simulator worker threads : ";
		// Stop all workers
		for(unsigned int index = 0; index < this->workersCount; index++)
		{
			// Stop worker
			this->workers[index]->Stop();
			std::cout << index << " ";
		}
		std::cout << "done!" << std::endl;
	}

	void SysWorkers::Enqueue(SysWorkItem* item)
	{
		// Enqueue a work item on the current worker
		this->workers[this->workerLast]->Enqueue(item);

		// Select next worker
		this->workerLast = (++this->workerLast) % this->workersCount;
	}
}
