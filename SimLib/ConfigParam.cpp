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
#include "ConfigParam.h"

namespace SimLib
{
	ConfigParam::ConfigParam(const char* name, std::string& line)
	{
		// Copy the line into a string
		this->lineSize = line.size()+1;
		this->line = alloc char[this->lineSize];
		this->lineTmp = alloc char[this->lineSize];
#ifdef _MSC_VER
		strcpy_s(this->line, this->lineSize, line.c_str());
#else
		strcpy(this->line, line.c_str());
#endif

		// Parse the string
		this->Parse((char*)name);
	}

	ConfigParam::ConfigParam(const ConfigParam& param)
	{
		this->lineSize = param.lineSize;
		this->line = alloc char[this->lineSize];
		this->lineTmp = alloc char[this->lineSize];
#ifdef _MSC_VER
		strcpy_s(this->line, this->lineSize, param.line);
#else
		strcpy(this->line, param.line);
#endif

		// Parse the string
		this->Parse(param.name);
	}

	ConfigParam::~ConfigParam()
	{
		delete[] this->line;
		delete[] this->lineTmp;
		delete[] this->values;
	}

	void ConfigParam::Parse(char* name)
	{
#ifdef _MSC_VER
		// If platform is Visual Studio, use the secure version of string tokenizer: strtok_s
		char* nextToken = NULL;
#endif

		// Split the line into tokens
#ifdef _MSC_VER
		strcpy_s(this->lineTmp, this->lineSize, this->line);
		char* token = strtok_s(this->lineTmp, " ,;:=\n\r\t", &nextToken);
#else
		strcpy(this->lineTmp, this->line);
		char* token = strtok(this->lineTmp, " ,;:=\n\r\t");
#endif
			
		// Check the first token is the parameter
		assert(0 == strcmp(name, token));
		this->name = token;

		// Set the values count to zero
		this->count = 0;

		// Iterate to count the values
		do
		{
#ifdef _MSC_VER
			token = strtok_s(NULL, " ,;:=\n\r\t", &nextToken);
#else
			token = strtok(NULL, " ,;:=\n\r\t");
#endif
			// If the token exists, increment the count
			if(NULL != token) this->count++;
		}
		while(NULL != token);

		// Allocate the list of values
		this->values = alloc char*[this->count];

#ifdef _MSC_VER
		strcpy_s(this->lineTmp, this->lineSize, this->line);
		token = strtok_s(this->lineTmp, " ,;:=\n\r\t", &nextToken);
#else
		strcpy(this->lineTmp, this->line);
		token = strtok(this->lineTmp, " ,;:=\n\r\t");
#endif

		for(uint index = 0; index < this->count; index++)
		{
#ifdef _MSC_VER
			token = strtok_s(NULL, " ,;:=\n\r\t", &nextToken);
#else
			token = strtok(NULL, " ,;:=\n\r\t");
#endif
			this->values[index] = token;
		}
	}
}
