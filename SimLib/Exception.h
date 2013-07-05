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

#define EXCEPTION_MESSAGE_SIZE	1024

namespace SimLib
{
	class Exception : public std::exception
	{
	protected:	
		const char* file;
		uint		line;
		char		message[EXCEPTION_MESSAGE_SIZE];

	public:
		Exception(
			const char*	file,
			uint		line,
			const char*	format = "",
			...
			) throw();
		Exception(const Exception& ex) throw();
		virtual ~Exception() throw() { }

		virtual const char* what() const throw() { return this->message; }
		inline const char*	Message() throw() { return this->message; }
		inline const char*	File() throw() { return this->file; }
		inline uint			Line() throw() { return this->line; }
	};
}
