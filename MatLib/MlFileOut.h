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

#include "MlFile.h"
#include "MlFileBinaryOut.h"

namespace MatLib
{
	class MlFileOut : public MlFile
	{
	private:
#ifdef _MSC_VER
		MATFile*		file;
#elif __GNUC__
		MlFileBinaryOut	file;
#endif

	public:
#ifdef _MSC_VER
		MlFileOut(const char* fileName);
#elif __GNUC__
		MlFileOut(const char* fileName) : file(fileName) { }
#endif
		virtual ~MlFileOut();

		void	Write(const char* name, MlArray* mlArray);
		void	Write(const char* name, MlArrayDouble* mlArray);
		void	Write(const char* name, MlArrayLogical* mlArray);
		void	Write(const char* name, double value);
	};
}
