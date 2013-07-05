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

#include "MlArray.h"

namespace MatLib
{
	class MlArrayLogical : public MlArray
	{
	private:
#if __GNUC__
		bool*			logicals;
#endif

	public:
		MlArrayLogical(MlSize ndim, const MlSize* dims, bool* logicals = NULL);
		MlArrayLogical(const MlArrayLogical& obj);
		virtual ~MlArrayLogical();

#ifdef _MSC_VER
		inline bool*			Logicals() { return mxGetLogicals(this->mlArray); }
#elif __GNUC__
		inline bool*			Logicals() { return this->logicals; }
#endif
	};
}
