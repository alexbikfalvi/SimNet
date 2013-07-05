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

#include "Ptr.h"
#include "ExceptionPtr.h"

namespace SimLib
{
	template<typename T> class PtrVector : public Ptr<T>
	{
	public:
		typedef PtrVector<T>							PtrBase;
		typedef typename std::vector<T>					VectorType;
		typedef typename PtrBase::VectorType*			VectorPtr;
		typedef typename PtrBase::VectorType&			VectorRef;
		typedef typename PtrBase::VectorType::size_type	SizeType;

	private:
		typename PtrBase::VectorPtr						vptr;
		typename PtrBase::SizeType						index;

	public:
		PtrVector() : vptr(0), index((SizeType)(-1)) { }
		PtrVector(VectorRef vref, SizeType index) : vptr(&vref), index(index) { }
		PtrVector(VectorRef vref, T* ptr) : vptr(&vref), index(ptr - &vref[0]) { }
		virtual ~PtrVector() { }

		virtual inline T&			operator*() { return (*this->vptr)[this->index]; }
		virtual inline T*			operator->() { return &(*this->vptr)[this->index]; }
		friend inline bool			operator<(const PtrVector<T>& left, const PtrVector<T>& right) { if(left.vptr != right.vptr) throw ExceptionPtr(__FILE__, __LINE__, "Cannot compare two pointers to different vectors."); return left.index < right.index; }
		friend inline bool			operator==(const PtrVector<T>& left, const PtrVector<T>& right) { return (left.vptr == right.vptr) && (left.index == right.index); }
		friend inline bool			operator!=(const PtrVector<T>& left, const PtrVector<T>& right) { return (left.vptr != right.vptr) || (left.index != right.index); }
		inline void					operator=(const PtrVector<T>& right) { this->vptr = right.vptr; this->index = right.index; }

		virtual inline bool			IsNull() { return (this->vptr == NULL) || (this->index == (SizeType)(-1)); }
		virtual inline bool			IsNotNull() { return (this->vptr != NULL) && (this->index != (SizeType)(-1)); }
		static inline bool			IsNull(SizeType index) { return (index == (SizeType)(-1)); }
		static inline bool			IsNotNull(SizeType index) { return (index != (SizeType)(-1)); }

		inline SizeType				Index() { return this->IsNotNull()?this->index:(SizeType)(-1); }

		static PtrVector<T>			Null() { return PtrVector<T>(); }
		static SizeType				NullIndex() { return (SizeType)(-1); }
	};
}
