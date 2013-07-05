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

#include <list>
#include "Delegate.h"

namespace SimLib
{
	template<typename R> class Event0
	{
	private:
		std::list<IDelegate0<R>*>	delegates;

	public:
		Event0() { }
		virtual ~Event0() { }

		void	operator+=(IDelegate0<R>& del);
		void	operator()();
	};

	template<typename R, typename P1> class Event1
	{
	private:
		std::list<IDelegate1<R, P1>*>	delegates;

	public:
		Event1() { }
		virtual ~Event1() { }

		void	operator+=(IDelegate1<R, P1>& del);
		void	operator()(P1 p1);
	};

	template<typename R, typename P1, typename P2> class Event2
	{
	private:
		std::list<IDelegate2<R, P1, P2>*>	delegates;

	public:
		Event2() { }
		virtual ~Event2() { }

		void	operator+=(IDelegate2<R, P1, P2>& del);
		void	operator()(P1 p1, P2 p2);
	};

	template<typename R, typename P1, typename P2, typename P3> class Event3
	{
	private:
		std::list<IDelegate3<R, P1, P2, P3>*>	delegates;

	public:
		Event3() { }
		virtual ~Event3() { }

		void	operator+=(IDelegate3<R, P1, P2, P3>& del);
		void	operator()(P1 p1, P2 p2, P3 p3);
	};

	template<typename R, typename P1, typename P2, typename P3, typename P4> class Event4
	{
	private:
		std::list<IDelegate4<R, P1, P2, P3, P4>*>	delegates;

	public:
		Event4() { }
		virtual ~Event4() { }

		void	operator+=(IDelegate4<R, P1, P2, P3, P4>& del);
		void	operator()(P1 p1, P2 p2, P3 p3, P4 p4);
	};

	template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5> class Event5
	{
	private:
		std::list<IDelegate5<R, P1, P2, P3, P4, P5>*>	delegates;

	public:
		Event5() { }
		virtual ~Event5() { }

		void	operator+=(IDelegate5<R, P1, P2, P3, P4, P5>& del);
		void	operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5);
	};

	template<typename R> void Event0<R>::operator +=(IDelegate0<R>& del)
	{
		this->delegates.push_back(&del);
	}

	template<typename R> void Event0<R>::operator ()()
	{
		for(typename std::list<IDelegate0<R>*>::iterator iter = this->delegates.begin(); iter != this->delegates.end(); iter++)
			(**iter)();
	}

	template<typename R, typename P1> void Event1<R, P1>::operator +=(IDelegate1<R, P1>& del)
	{
		this->delegates.push_back(&del);
	}

	template<typename R, typename P1> void Event1<R, P1>::operator ()(P1 p1)
	{
		for(typename std::list<IDelegate1<R, P1>*>::iterator iter = this->delegates.begin(); iter != this->delegates.end(); iter++)
			(**iter)(p1);
	}

	template<typename R, typename P1, typename P2> void Event2<R, P1, P2>::operator +=(IDelegate2<R, P1, P2>& del)
	{
		this->delegates.push_back(&del);
	}

	template<typename R, typename P1, typename P2> void Event2<R, P1, P2>::operator ()(P1 p1, P2 p2)
	{
		for(typename std::list<IDelegate2<R, P1, P2>*>::iterator iter = this->delegates.begin(); iter != this->delegates.end(); iter++)
			(**iter)(p1, p2);
	}

	template<typename R, typename P1, typename P2, typename P3> void Event3<R, P1, P2, P3>::operator +=(IDelegate3<R, P1, P2, P3>& del)
	{
		this->delegates.push_back(&del);
	}

	template<typename R, typename P1, typename P2, typename P3> void Event3<R, P1, P2, P3>::operator ()(P1 p1, P2 p2, P3 p3)
	{
		for(typename std::list<IDelegate3<R, P1, P2, P3>*>::iterator iter = this->delegates.begin(); iter != this->delegates.end(); iter++)
			(**iter)(p1, p2, p3);
	}

	template<typename R, typename P1, typename P2, typename P3, typename P4> void Event4<R, P1, P2, P3, P4>::operator +=(IDelegate4<R, P1, P2, P3, P4>& del)
	{
		this->delegates.push_back(&del);
	}

	template<typename R, typename P1, typename P2, typename P3, typename P4> void Event4<R, P1, P2, P3, P4>::operator ()(P1 p1, P2 p2, P3 p3, P4 p4)
	{
		for(typename std::list<IDelegate4<R, P1, P2, P3, P4>*>::iterator iter = this->delegates.begin(); iter != this->delegates.end(); iter++)
			(**iter)(p1, p2, p3, p4);
	}

	template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5> void Event5<R, P1, P2, P3, P4, P5>::operator +=(IDelegate5<R, P1, P2, P3, P4, P5>& del)
	{
		this->delegates.push_back(&del);
	}

	template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5> void Event5<R, P1, P2, P3, P4, P5>::operator ()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		for(typename std::list<IDelegate5<R, P1, P2, P3, P4, P5>*>::iterator iter = this->delegates.begin(); iter != this->delegates.end(); iter++)
			(**iter)(p1, p2, p3, p4, p5);
	}
}