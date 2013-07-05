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

#pragma once

namespace SimLib
{
	template<typename R> class IDelegate0
	{
	public:
		IDelegate0() { }
		virtual ~IDelegate0() { }

		virtual R	operator()() = 0;
	};

	template<typename R, typename P1> class IDelegate1
	{
	public:
		IDelegate1() { }
		virtual ~IDelegate1() { }

		virtual R	operator()(P1) = 0;
	};

	template<typename R, typename P1, typename P2> class IDelegate2
	{
	public:
		IDelegate2() { }
		virtual ~IDelegate2() { }

		virtual R	operator()(P1, P2) = 0;
	};

	template<typename R, typename P1, typename P2, typename P3> class IDelegate3
	{
	public:
		IDelegate3() { }
		virtual ~IDelegate3() { }

		virtual R	operator()(P1, P2, P3) = 0;
	};

	template<typename R, typename P1, typename P2, typename P3, typename P4> class IDelegate4
	{
	public:
		IDelegate4() { }
		virtual ~IDelegate4() { }

		virtual R	operator()(P1, P2, P3, P4) = 0;
	};

	template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5> class IDelegate5
	{
	public:
		IDelegate5() { }
		virtual ~IDelegate5() { }

		virtual R	operator()(P1, P2, P3, P4, P5) = 0;
	};

	template<typename T, typename R> class Delegate0 : public IDelegate0<R>
	{
	private:
		T&	object;
		R	(T::*func)();

	public:
		Delegate0(T& object, R (T::*func)()) : object(object), func(func) { }
		virtual ~Delegate0() { }

		virtual R	operator()()
		{
			return (this->object.*this->func)();
		}
	};

	template<typename T, typename R, typename P1> class Delegate1 : public IDelegate1<R, P1>
	{
	private:
		T&	object;
		R	(T::*func)(P1);

	public:
		Delegate1(T& object, R (T::*func)(P1)) : object(object), func(func) { }
		virtual ~Delegate1() { }

		virtual R	operator()(P1 p1)
		{
			return (this->object.*this->func)(p1);
		}
	};

	template<typename T, typename R, typename P1, typename P2> class Delegate2 : public IDelegate2<R, P1, P2>
	{
	private:
		T&	object;
		R	(T::*func)(P1, P2);

	public:
		Delegate2(T& object, R (T::*func)(P1, P2)) : object(object), func(func) { }
		virtual ~Delegate2() { }

		virtual R	operator()(P1 p1, P2 p2)
		{
			return (this->object.*this->func)(p1, p2);
		}
	};

	template<typename T, typename R, typename P1, typename P2, typename P3> class Delegate3 : public IDelegate3<R, P1, P2, P3>
	{
	private:
		T&	object;
		R	(T::*func)(P1, P2, P3);

	public:
		Delegate3(T& object, R (T::*func)(P1, P2, P3)) : object(object), func(func) { }
		virtual ~Delegate3() { }

		virtual R	operator()(P1 p1, P2 p2, P3 p3)
		{
			return (this->object.*this->func)(p1, p2, p3);
		}
	};

	template<typename T, typename R, typename P1, typename P2, typename P3, typename P4> class Delegate4 : public IDelegate4<R, P1, P2, P3, P4>
	{
	private:
		T&	object;
		R	(T::*func)(P1, P2, P3, P4);

	public:
		Delegate4(T& object, R (T::*func)(P1, P2, P3, P4)) : object(object), func(func) { }
		virtual ~Delegate4() { }

		virtual R	operator()(P1 p1, P2 p2, P3 p3, P4 p4)
		{
			return (this->object.*this->func)(p1, p2, p3, p4);
		}
	};

	template<typename T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5> class Delegate5 : public IDelegate5<R, P1, P2, P3, P4, P5>
	{
	private:
		T&	object;
		R	(T::*func)(P1, P2, P3, P4, P5);

	public:
		Delegate5(T& object, R (T::*func)(P1, P2, P3, P4, P5)) : object(object), func(func) { }
		virtual ~Delegate5() { }

		virtual R	operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
		{
			return (this->object.*this->func)(p1, p2, p3, p4, p5);
		}
	};
}