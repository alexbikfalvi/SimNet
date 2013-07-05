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
#include "IoStreamOutConsole.h"

#if defined(_MSC_VER)
#include <windows.h>
#endif

namespace SimLib
{
	IoStreamOutConsole IoStreamOutConsole::stream;

	IoStreamOut& IoStreamOutConsole::operator<<(Color color)
	{
#if defined(_MSC_VER)
		HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
		switch(color)
		{
		case IoStreamOut::BLACK: SetConsoleTextAttribute(console, 0); break;
		case IoStreamOut::DARK_BLUE: SetConsoleTextAttribute(console, FOREGROUND_BLUE); break;
		case IoStreamOut::DARK_GREEN: SetConsoleTextAttribute(console, FOREGROUND_GREEN); break;
		case IoStreamOut::DARK_CYAN: SetConsoleTextAttribute(console, FOREGROUND_BLUE | FOREGROUND_GREEN); break;
		case IoStreamOut::DARK_RED: SetConsoleTextAttribute(console, FOREGROUND_RED); break;
		case IoStreamOut::DARK_MAGENTA: SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_BLUE); break;
		case IoStreamOut::DARK_YELLOW: SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN); break;
		case IoStreamOut::DARK_GRAY: SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); break;
		case IoStreamOut::LIGHT_GRAY: SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY); break;
		case IoStreamOut::LIGHT_BLUE: SetConsoleTextAttribute(console, FOREGROUND_BLUE | FOREGROUND_INTENSITY); break;
		case IoStreamOut::LIGHT_GREEN: SetConsoleTextAttribute(console, FOREGROUND_GREEN | FOREGROUND_INTENSITY); break;
		case IoStreamOut::LIGHT_CYAN: SetConsoleTextAttribute(console, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY); break;
		case IoStreamOut::LIGHT_RED: SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_INTENSITY); break;
		case IoStreamOut::LIGHT_MAGENTA: SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY); break;
		case IoStreamOut::LIGHT_YELLOW: SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY); break;
		case IoStreamOut::WHITE: SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY); break;
		case IoStreamOut::CLEAR: SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY); break;
		}
		
#elif defined(__GNUC__)
		std::cout << "\033[" << (uint)color << "m" << std::flush;
#endif
		return *this;
	}
}