
//==============================================================================
//
// utils.h : utilities like json parse and file downloads
//
//==============================================================================
//  Copyright (C) Guilaume Plante 2020 <cybercastor@icloud.com>
//==============================================================================


#ifndef __UTILS_H__
#define __UTILS_H__

#include <vector>
#include <string>
#include "httplib.h"
#include <iostream>


enum class TrackerType {
    All,        // Represents all tracker types
    UDP,        // UDP-based trackers
    WinSock,    // Trackers using Windows Sockets (Winsock)
    IP,         // Trackers using direct IP communication
    Archived,    // Archived or deprecated trackers
	Best,
	Test
};


// Function to convert string to TrackerType enum
std::string get_current_datetime();
void banner();
void usage();
bool isNumeric(const std::string& str);
void usage_error(std::string message);
void app_error(std::string message);
std::string log(const httplib::Request& req, const httplib::Response& res);
int write_content(const char* filename, const char* buffer, int size);
std::string download_file_content(std::string dwl_url, bool optVerbose = false);
bool download_file(const std::string& url, const std::string& output_path, bool optVerbose);
bool download_file_ext(std::string fullUrl, std::string outFile, bool optVerbose);

std::vector<std::string> split(const std::string& s, char delim);
std::string dump_headers(const httplib::Headers& headers);
std::string dump_params(const httplib::Params& params);

bool get_temp_file_path(std::string& tmp_path);
std::string extract_scheme_and_host(const std::string& url);
std::string generate_guid();
std::ostringstream read_file_to_stream(const std::string& filePath);

namespace Convert
{

	// The following 2 functions (allocate_argn/release_argn) with utils wstrlen/dup are used to convert WSTR (unicode) Main Argv parameter to char. 
	// This is for example used for cmdline parsing
	int wstrlen(_TCHAR* wstr);
	char* wstrdup(_TCHAR* wSrc);
	char** allocate_argn(int argc, _TCHAR* argv[]);
	void release_argn(int argc, char** nargv);


	LPWSTR StringToString(LPCSTR str);
	LPSTR StringToString(LPCWSTR str);

	LPWSTR Int32ToString(__int32 value, int base = 10);
	LPWSTR UInt32ToString(unsigned __int32 value, int base = 10);
	LPWSTR Int64ToString(__int64 value, int base = 10);
	LPWSTR UInt64ToString(unsigned __int64 value, int base = 10);
	LPWSTR FloatToString(float value);
	LPWSTR DoubleToString(double value);

	__int32 StringToInt32(LPCWSTR str, int base = 10);
	unsigned __int32 StringToUInt32(LPCWSTR str, int base = 10);
	__int64 StringToInt64(LPCWSTR str, int base = 10);
	unsigned __int64 StringToUInt64(LPCWSTR str, int base = 10);
	float StringToFloat(LPCWSTR str);
	double StringToDouble(LPCWSTR str);

	LPWSTR UInt32ToHexString(unsigned __int32 value);
	LPWSTR UInt64ToHexString(unsigned __int64 value);
	LPWSTR BytesToHexView(LPBYTE data, DWORD length);
}


#endif//__UTILS_H__