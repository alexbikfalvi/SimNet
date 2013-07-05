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
#include "Config.h"

namespace SimLib
{
	Config::Config(const char* fileName)
	{
		// Open the file
		std::ifstream file(fileName);

		try
		{
			// Create a string stream buffer
			std::stringstream	buffer;
			// Read the file in the buffer
			buffer << file.rdbuf();
			// Save the buffer into a string
			this->str = std::string(buffer.str());
		}
		catch(...)
		{
			// Close the file
			file.close();
			// Rethrow exception
			throw;
		}
		// Close the file
		file.close();
	}

	Config::Config(const Config& config)
	{
		this->str = config.str;
	}

	bool Config::Has(const char* name)
	{
		return std::string::npos != this->str.find(name);
	}

	ConfigParam Config::operator[](const char* name)
	{
		// Get the length of name
		size_t len = strlen(name);
		
		size_t posFirst = 0;
		
		do
		{
			// Search the string for the parameter, starting from the beginning of the string
			posFirst = this->str.find(name, posFirst);
		
			// If the parameter is not found throw a configuration exception
			if(std::string::npos == posFirst) throw ExceptionConfig(__FILE__, __LINE__, name, "Configuration parameter not found");

			// Check that a delimiter (but not end of line) is found after the name
			if(this->str.find_first_of(" ,;:=\t", posFirst+1) == posFirst+len)
			{
				// Find the end of the current line or first comment
				size_t posLast = this->str.find_first_of("\n\r/", posFirst);
				if(std::string::npos != posLast) posLast -= posFirst;

				// Get the parameter line
				std::string line = this->str.substr(posFirst, posLast);

				// Return a config parameter for this line
				return ConfigParam(name, line);
			}

			// Increment the position to search
			posFirst += len;
		}
		while(true);
	}
}
