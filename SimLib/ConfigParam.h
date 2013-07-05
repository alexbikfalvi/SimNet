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

#include "ExceptionConfig.h"

namespace SimLib
{
	class ConfigParam
	{
	private:
		uint	count;
		char*	line;
		char*	lineTmp;
		uint	lineSize;
		char*	name;
		char**	values;

	public:
		ConfigParam(const char* name, std::string& line);
		ConfigParam(const ConfigParam& param);
		virtual ~ConfigParam();

		inline uint								Count() { return this->count; }
		inline char*							Name() { return this->name; }
		template <typename T> T					Value(uint index = 0)
		{
			if((index >= this->count) || (NULL == this->values[index]))
				throw ExceptionConfigIndex(__FILE__, __LINE__, this->name, index, "A value at specified index does not exist.");

			T value;
			this->Convert(this->values[index], value);
			return value;
		}
		template <typename T> std::vector<T>	Values()
		{
			std::vector<T> values;
			values.resize(this->count);

			for(uint index = 0; index < this->count; index++)
			{
				T value;
				this->Convert(this->values[index], value);
				values[index] = value;
			}
			return values;
		}

	private:
		void									Parse(char* name);

		inline void								Convert(const char* str, uint& value) { value = (uint)atoi(str); }
		inline void								Convert(const char* str, int& value) { value = atoi(str); }
		inline void								Convert(const char* str, double& value) { value = atof(str); }
		inline void								Convert(const char*	str, std::string& value) { value = std::string(str); }
		inline void								Convert(const char* str, bool& value) { value = (atoi(str) != 0); }
	};
}
