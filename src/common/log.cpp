
//==============================================================================
//
//  log.cpp
//
//==============================================================================
//  Copyright (C)  Guillaume Plante <codegp@icloud.com>
//==============================================================================



#include "stdafx.h"
#include "log.h"
#include "config.h"
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <wtypes.h>



extern bool g_is_verbose;



void __cdecl LogNoticeFunc(const char* format, ...)
{
	const std::string CONSOLE_COLOR_RED = "\u001b[31m";
	char    buf[4096], * p = buf;
	va_list args;
	int     n;

	va_start(args, format);
	n = vsnprintf(p, sizeof buf - 3, format, args); // buf-3 is room for CR/LF/NUL
	va_end(args);

	p += (n < 0) ? sizeof buf - 3 : n;

	while (p > buf && isspace(p[-1]))
		*--p = '\0';

	*p++ = '\r';
	*p++ = '\n';
	*p = '\0';

	std::string text(buf);
	int textLength = text.size();

	// Build horizontal border (width = text length + 2 for padding)
	std::string horizontal(textLength + 2, '=');

	// Construct the three lines of the box using the specified box-drawing characters
	std::string topBorder = "+" + horizontal + "+";
	std::string middleLine = "| " + text + " |";
	std::string bottomBorder = "+" + horizontal + "+";

	// Output the box with the border in RED and the text in YELLOW.
	std::clog << ANSI_TEXT_COLOR_RED << topBorder << ANSI_TEXT_COLOR_RESET << "\r\n"
		<< ANSI_TEXT_COLOR_RED << "| " << ANSI_TEXT_COLOR_YELLOW << text << ANSI_TEXT_COLOR_RED << " |" << ANSI_TEXT_COLOR_RESET << "\r\n"
		<< ANSI_TEXT_COLOR_RED << bottomBorder << ANSI_TEXT_COLOR_RESET << "\r\n";

}



void __cdecl ErrorDescFunc(const char* channel, const char* format, ...)
{
	const std::string CONSOLE_COLOR_RED = "\u001b[31m";
	char    buf[4096], * p = buf;
	va_list args;
	int     n;

	va_start(args, format);
	n = vsnprintf(p, sizeof buf - 3, format, args); // buf-3 is room for CR/LF/NUL
	va_end(args);

	p += (n < 0) ? sizeof buf - 3 : n;

	while (p > buf && isspace(p[-1]))
		*--p = '\0';

	*p++ = '\r';
	*p++ = '\n';
	*p = '\0';


	LOG_ERROR(channel, "%s", buf);

		EndOfLineEscapeTag FormatDebugTitle{ RED_UNDERLINED, ANSI_TEXT_COLOR_RESET };
		EndOfLineEscapeTag FormatDebug1{ RED_UNDERLINED, ANSI_TEXT_COLOR_RESET };
		EndOfLineEscapeTag FormatDebug2{ CONSOLE_COLOR_YELLOW, ANSI_TEXT_COLOR_RESET };
		EndOfLineEscapeTag FormatDebug3{ CONSOLE_COLOR_BKGRND_YELLOW_RED , ANSI_TEXT_COLOR_RESET };
		std::clog << FormatDebugTitle << "ERROR";
		std::clog << FormatDebug2 << " [" << channel << "]";
		std::clog << FormatDebug3 << " " << buf;




}


void __cdecl DebugTraceFunc(const char* channel, const char* format, ...)
{
	const std::string CONSOLE_COLOR_RED = "\u001b[31m";
	char    buf[4096], * p = buf;
	va_list args;
	int     n;

	va_start(args, format);
	n = vsnprintf(p, sizeof buf - 3, format, args); // buf-3 is room for CR/LF/NUL
	va_end(args);

	p += (n < 0) ? sizeof buf - 3 : n;

	while (p > buf && isspace(p[-1]))
		*--p = '\0';

	*p++ = '\r';
	*p++ = '\n';
	*p = '\0';

	
	LOG_TRACE(channel, "%s", buf);

	if (CONFIG.isDebugEnabled()) {
		
		EndOfLineEscapeTag FormatDebug1{ RED_UNDERLINED, ANSI_TEXT_COLOR_RESET };
		EndOfLineEscapeTag FormatDebug2{ CONSOLE_COLOR_BKGRND_WHITE_GREEN, ANSI_TEXT_COLOR_RESET };
		EndOfLineEscapeTag FormatDebug3{ CONSOLE_COLOR_YELLOW, ANSI_TEXT_COLOR_RESET };
		std::clog << FormatDebug2 << "[" << channel << "]";
		std::clog << FormatDebug3 << " " << buf;
	}
}


void __cdecl LogAppMsg(const char* format, ...)
{
	const std::string CONSOLE_COLOR_RED = "\u001b[31m";
	char    buf[4096], * p = buf;
	va_list args;
	int     n;

	va_start(args, format);
	n = vsnprintf(p, sizeof buf - 3, format, args); // buf-3 is room for CR/LF/NUL
	va_end(args);

	p += (n < 0) ? sizeof buf - 3 : n;

	while (p > buf && isspace(p[-1]))
		*--p = '\0';

	*p++ = '\r';
	*p++ = '\n';
	*p = '\0';
	LOG_WARNING("application::message", "%s", buf);
	EndOfLineEscapeTag FormatDebug1{ CYAN_UNDERLINED_B, ANSI_TEXT_COLOR_RESET };

	std::clog << FormatDebug1  << buf;
}


void __cdecl LogConfigFunc(const char* format, ...)
{
	const std::string CONSOLE_COLOR_RED = "\u001b[31m";
	char    buf[4096], * p = buf;
	va_list args;
	int     n;

	va_start(args, format);
	n = vsnprintf(p, sizeof buf - 3, format, args); // buf-3 is room for CR/LF/NUL
	va_end(args);

	p += (n < 0) ? sizeof buf - 3 : n;

	while (p > buf && isspace(p[-1]))
		*--p = '\0';

	*p++ = '\r';
	*p++ = '\n';
	*p = '\0';

	EndOfLineEscapeTag FormatDebug1{ ANSI_TEXT_COLOR_BLUE_BRIGHT, ANSI_TEXT_COLOR_RESET };
	EndOfLineEscapeTag FormatDebug2{ CONSOLE_COLOR_CYAN, ANSI_TEXT_COLOR_RESET };

	std::clog << FormatDebug1 << "[configuration] " << buf;
}


void __cdecl LogInfoFunc(const char* format, ...)
{
	const std::string CONSOLE_COLOR_RED = "\u001b[31m";
	char    buf[4096], * p = buf;
	va_list args;
	int     n;

	va_start(args, format);
	n = vsnprintf(p, sizeof buf - 3, format, args); // buf-3 is room for CR/LF/NUL
	va_end(args);

	p += (n < 0) ? sizeof buf - 3 : n;

	while (p > buf && isspace(p[-1]))
		*--p = '\0';

	*p++ = '\r';
	*p++ = '\n';
	*p = '\0';

	EndOfLineEscapeTag FormatDebug1{ RED_UNDERLINED, ANSI_TEXT_COLOR_RESET };
	EndOfLineEscapeTag FormatDebug2{ CONSOLE_COLOR_CYAN, ANSI_TEXT_COLOR_RESET };
	EndOfLineEscapeTag FormatDebug3{ ANSI_TEXT_COLOR_WHITE_BRIGHT, ANSI_TEXT_COLOR_RESET };
	std::clog << FormatDebug2 << " [I] ";
	std::clog << FormatDebug3 << " " << buf;
}

void __cdecl LogInfoFunc2(const char* channel, const char* format, ...)
{
	const std::string CONSOLE_COLOR_RED = "\u001b[31m";
	char    buf[4096], * p = buf;
	va_list args;
	int     n;

	va_start(args, format);
	n = vsnprintf(p, sizeof buf - 3, format, args); // buf-3 is room for CR/LF/NUL
	va_end(args);

	p += (n < 0) ? sizeof buf - 3 : n;

	while (p > buf && isspace(p[-1]))
		*--p = '\0';

	*p++ = '\r';
	*p++ = '\n';
	*p = '\0';


	LOG_TRACE(channel, "%s", buf);

	EndOfLineEscapeTag FormatDebug1{ RED_UNDERLINED, ANSI_TEXT_COLOR_RESET };
	EndOfLineEscapeTag FormatDebug2{ CONSOLE_COLOR_CYAN, ANSI_TEXT_COLOR_RESET };
	EndOfLineEscapeTag FormatDebug3{ ANSI_TEXT_COLOR_WHITE_BRIGHT, ANSI_TEXT_COLOR_RESET };
	std::clog << FormatDebug2 << " [I] ";
	std::clog << FormatDebug3 << " " << buf;
}

void __cdecl ConsoleVerboseOut(std::string color, const char* format, ...)
{
	if (!g_is_verbose) { return; }
	char    buf[4096], * p = buf;
	va_list args;
	int     n;

	va_start(args, format);
	n = vsnprintf(p, sizeof buf - 3, format, args); // buf-3 is room for CR/LF/NUL
	va_end(args);

	p += (n < 0) ? sizeof buf - 3 : n;

	while (p > buf && isspace(p[-1]))
		*--p = '\0';

	*p++ = '\r';
	*p++ = '\n';
	*p = '\0';


	EndOfLineEscapeTag Format{ color, ANSI_TEXT_COLOR_RESET };
	std::clog << Format << buf;
}

void __cdecl ConsoleTrace(std::string color, const char* format, ...)
{
	char    buf[4096], * p = buf;
	va_list args;
	int     n;

	va_start(args, format);
	n = vsnprintf(p, sizeof buf - 3, format, args); // buf-3 is room for CR/LF/NUL
	va_end(args);

	p += (n < 0) ? sizeof buf - 3 : n;

	while (p > buf && isspace(p[-1]))
		*--p = '\0';

	*p++ = '\r';
	*p++ = '\n';
	*p = '\0';


	EndOfLineEscapeTag Format{ color, ANSI_TEXT_COLOR_RESET };
	std::clog << Format << buf;
}


//==============================================================================
// ConsoleOut
// Used by the ServiceTerminal
//==============================================================================
void __cdecl ConsoleOut(std::string color, const char *format, ...)
{
	char    buf[4096], *p = buf;
	va_list args;
	int     n;

	va_start(args, format);
	n = vsnprintf(p, sizeof buf - 3, format, args); // buf-3 is room for CR/LF/NUL
	va_end(args);

	p += (n < 0) ? sizeof buf - 3 : n;

	while (p > buf  &&  isspace(p[-1]))
		*--p = '\0';

	*p++ = '\r';
	*p++ = '\n';
	*p = '\0';


	EndOfLineEscapeTag Format{ color, ANSI_TEXT_COLOR_RESET };
	std::clog << Format << buf;
}

void __cdecl ConsoleOutNoRl(std::string color, const char *format, ...)
{
	char    buf[4096], *p = buf;
	va_list args;
	int     n;

	va_start(args, format);
	n = vsnprintf(p, sizeof buf - 3, format, args); // buf-3 is room for CR/LF/NUL
	va_end(args);

	p += (n < 0) ? sizeof buf - 3 : n;

	while (p > buf  &&  isspace(p[-1]))
		*--p = '\0';

	*p++ = ' ';
	
	*p = '\0';


	EndOfLineEscapeTag Format{ color, ANSI_TEXT_COLOR_RESET };
	std::clog << Format << buf;
}

void __cdecl ConsoleDebug(std::string color, const char* format, ...)
{
	if (!CONFIG.isDebugEnabled()) {
		return;
	}
	char    buf[4096], * p = buf;
	va_list args;
	int     n;

	va_start(args, format);
	n = vsnprintf(p, sizeof buf - 3, format, args); // buf-3 is room for CR/LF/NUL
	va_end(args);

	p += (n < 0) ? sizeof buf - 3 : n;

	while (p > buf && isspace(p[-1]))
		*--p = '\0';

	*p++ = ' ';

	*p = '\0';

	

	EndOfLineEscapeTag FormatText{ CONSOLE_COLOR_YELLOW_BRIGHT, ANSI_TEXT_COLOR_RESET };
	EndOfLineEscapeTag FormatDebug{ CONSOLE_COLOR_CYAN_BRIGHT, ANSI_TEXT_COLOR_RESET };
	EndOfLineEscapeTag FormatReset{ ANSI_TEXT_COLOR_BLACK, ANSI_TEXT_COLOR_RESET };
	std::clog << FormatDebug << "[debug] ";
	std::clog << FormatText << buf;
	std::clog << FormatReset << "\n";
}

void __cdecl ConsoleTrace(const char *format, ...)
{
	char    buf[4096], *p = buf;
	va_list args;
	int     n;

	va_start(args, format);
	n = vsnprintf(p, sizeof buf - 3, format, args); // buf-3 is room for CR/LF/NUL
	va_end(args);

	p += (n < 0) ? sizeof buf - 3 : n;

	while (p > buf  &&  isspace(p[-1]))
		*--p = '\0';

	//*p++ = '\r';
	//*p++ = '\n';
	*p = '\0';


	EndOfLineEscapeTag FormatText{ CONSOLE_COLOR_BKGRND_YELLOW, ANSI_TEXT_COLOR_RESET };
	EndOfLineEscapeTag FormatReset{ ANSI_TEXT_COLOR_BLACK, ANSI_TEXT_COLOR_RESET };
	std::clog << FormatText << buf;
	std::clog << FormatReset << "";
}
void __cdecl ConsoleProcess(unsigned int id,const char *name)
{
	char    buf[32], *p = buf;
	sprintf(buf, "[%5d]",id);	
	EndOfLineEscapeTag FormatId{ ANSI_TEXT_COLOR_BLUE_BRIGHT, ANSI_TEXT_COLOR_RESET };
	EndOfLineEscapeTag FormatName{ ANSI_TEXT_COLOR_WHITE, ANSI_TEXT_COLOR_RESET };

	std::clog << FormatId << p << "\t";
	std::clog << FormatName << name << "\n";
}
void __cdecl ConsoleLog(const char* format, ...)
{
	char    buf[4096], * p = buf;
	va_list args;
	int     n;

	va_start(args, format);
	n = vsnprintf(p, sizeof buf - 3, format, args); // buf-3 is room for CR/LF/NUL
	va_end(args);

	p += (n < 0) ? sizeof buf - 3 : n;

	while (p > buf && isspace(p[-1]))
		*--p = '\0';

	*p++ = ' ';

	*p = '\0';

	EndOfLineEscapeTag FormatId{ ANSI_TEXT_COLOR_MAGENTA_BRIGHT, ANSI_TEXT_COLOR_RESET };
	EndOfLineEscapeTag FormatName{ ANSI_TEXT_COLOR_YELLOW, ANSI_TEXT_COLOR_RESET };

	std::clog << FormatId << p << "\t";
	std::clog << FormatName << buf << "\n";


}
void __cdecl ConsoleProcessPath(unsigned int id,const char *name,const char *path)
{
	char    buf[32], *p = buf;
	sprintf(buf, "[%5d]",id);

	EndOfLineEscapeTag FormatId{ ANSI_TEXT_COLOR_BLUE_BRIGHT, ANSI_TEXT_COLOR_RESET };
	EndOfLineEscapeTag FormatName{ ANSI_TEXT_COLOR_WHITE, ANSI_TEXT_COLOR_RESET };
	EndOfLineEscapeTag FormatPath{ ANSI_TEXT_COLOR_BLACK_BRIGHT, ANSI_TEXT_COLOR_RESET };
	std::clog << FormatId << p << "\t";
	if(strlen(name)<8){
		std::clog << FormatName << name << "\t\t";
	}else if(strlen(name)>14){
		std::clog << FormatName << name << "\n\t\t";	
	}else{
		std::clog << FormatName << name << "\t";	
	
	}
	
	std::clog << FormatPath << path << "\n";
}
void __cdecl ConsoleTitle( const char *title, std::string color )
{
	EndOfLineEscapeTag FormatTitle{ color, ANSI_TEXT_COLOR_RESET };
	EndOfLineEscapeTag FormatName{ BLACK_UNDERLINED, ANSI_TEXT_COLOR_RESET };
	std::clog << FormatTitle << title;
	std::clog << FormatName << " ";
}
void __cdecl ConsoleInfo(const char *title, std::string color)
{
	EndOfLineEscapeTag FormatTitle{ color, ANSI_TEXT_COLOR_RESET };
	EndOfLineEscapeTag FormatName{ BLACK_UNDERLINED, ANSI_TEXT_COLOR_RESET };
	std::clog << FormatTitle << title;
	std::clog << FormatName << " ";
}


//==============================================================================
// SystemDebugOutput
// Kernel-mode and Win32 debug output
//   - Win32 OutputDebugString
//   - Kernel - mode DbgPrint
// You can monitor this stream using Debugview from SysInternals
// https://docs.microsoft.com/en-us/sysinternals/downloads/debugview
//==============================================================================
void __cdecl SystemDebugOutput(const wchar_t *channel, const char *format, ...)
{
#ifndef FINAL
	char    buf[4096], *p = buf;
	va_list args;
	int     n;

	va_start(args, format);
	n = vsnprintf(p, sizeof buf - 3, format, args); // buf-3 is room for CR/LF/NUL
	va_end(args);

	p += (n < 0) ? sizeof buf - 3 : n;

	while (p > buf  &&  isspace(p[-1]))
		*--p = '\0';

	*p++ = '\r';
	*p++ = '\n';
	*p = '\0';

	OutputDebugStringA(buf);
#ifdef KERNEL_DEBUG
	DbgPrint(buf);
#endif // KERNEL_DEBUG

#endif // FINAL
}
