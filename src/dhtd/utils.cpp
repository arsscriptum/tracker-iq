
//==============================================================================
//
// utils.cpp : utilities like json parse and file downloads
//
//==============================================================================
// Copyright (C)  Guillaume Plante <codegp@icloud.com>
//==============================================================================


#include "stdafx.h"
#include "utils.h"
#include <Windows.h>
#include <system_error>
#include <memory>
#include <string>
#include "log.h"
#include "httplib.h"

#include "Shlwapi.h"
#include "log.h"

#include "jsmn.h"
#include <codecvt>
#include <locale>

#include <unordered_map>
#include <iterator>
#include <regex>
#include <filesystem>
#include "version.h"

#include <sstream>
#include <cstdlib>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#ifdef USING_HTTPLIB
#pragma message( "USING HTTPLIB" )
#endif

using namespace httplib;
using namespace std;



std::string get_current_datetime() {
	// Get the current time using system_clock (NOT steady_clock)
	auto now = std::chrono::system_clock::now();

	// Convert to time_t (calendar time)
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);

	// Format time as YYYY-MM-DD HH:MM:SS
	std::stringstream ss;
	ss << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S");

	return ss.str();
}


vector<string> split(const string& s, char delim) {
	vector<string> result;
	stringstream ss(s);
	string item;

	while (getline(ss, item, delim)) {
		result.push_back(item);
	}

	return result;
}



string dump_headers(const Headers& headers) {
	string s;
	char buf[BUFSIZ];

	for (const auto& x : headers) {
		snprintf(buf, sizeof(buf), "%s: %s\n", x.first.c_str(), x.second.c_str());
		s += buf;
	}

	return s;
}

string dump_params(const Params& params) {
	string s;
	char buf[BUFSIZ];

	for (const auto& x : params) {
		snprintf(buf, sizeof(buf), "%s: %s\n", x.first.c_str(), x.second.c_str());
		s += buf;
	}

	return s;
}


void banner() {
	std::wcout << std::endl;
	std::string title = "dhtd v" + dhtd::version::GetAppVersion() + " - custom DHT server using libtorrent-rasterbar\n";
	COUTBANNER_1(title.c_str());

	COUTBANNER_2("Built on %s\n", __TIMESTAMP__);
	COUTBANNER_2("Copyright (C) 2000-2021 Guillaume Plante\n");
	std::wcout << std::endl;
}
void usage() {
	COUTBANNER_2("Usage: dhtd [-h][-d][-v][-n][-u] url [-p] path \n");
	COUTBANNER_2("   -v                    Verbose mode\n");
	COUTBANNER_2("   -h                    Help\n");
	COUTBANNER_2("   -n                    No banner\n");
	COUTBANNER_2("   -c, --config  <file>  Specify config  file\n");
	std::wcout << std::endl;
}

void usage_error(std::string message) {
	COUTRS("Error: %s\n", message.c_str());
	usage();
}


void app_error(std::string message) {
	COUTRS("Program Error: %s\n", message.c_str());
}





std::string log(const Request& req, const Response& res) {
	string s;
	char buf[BUFSIZ];

	s += "================================\n";

	snprintf(buf, sizeof(buf), "%s %s %s", req.method.c_str(),
		req.version.c_str(), req.path.c_str());
	s += buf;

	string query;
	for (auto it = req.params.begin(); it != req.params.end(); ++it) {
		const auto& x = *it;
		snprintf(buf, sizeof(buf), "%c%s=%s",
			(it == req.params.begin()) ? '?' : '&', x.first.c_str(),
			x.second.c_str());
		query += buf;
	}
	snprintf(buf, sizeof(buf), "%s\n", query.c_str());
	s += buf;

	s += dump_headers(req.headers);


	s += "--------------------------------\n";

	snprintf(buf, sizeof(buf), "%d\n", res.status);
	s += buf;
	s += dump_headers(res.headers);

	return s;
}




int write_content(const char* filename, const char* buffer, int size) {
	if (!filename || !buffer || size <= 0) {
		return -1; // Invalid arguments
	}

	std::ofstream file(filename, std::ios::binary);
	if (!file) {
		return -1; // Failed to open file
	}

	file.write(buffer, size);
	if (!file) {
		return -1; // Failed to write to file
	}

	return size; // Return number of bytes written
}








std::string download_file_content(string url, bool optVerbose) {

	bool result = false;
	string rcvdata;
	string dwl_url = "https://raw.githubusercontent.com";
	vector<string> v = split(url, '/');
	char* str = (char*)malloc(512);
	strcpy(str, "/");
	for (int i = 3; i < v.size(); i++) {
		strcat(str, v[i].c_str());
		if (i < (v.size() - 1)) { strcat(str, "/"); }
	}
	try {
		httplib::Headers h;

		h.emplace("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/99.0.4844.51 Safari/537.36");

		
		LOG_TRACE("download_file_content", "Requesting File to server %s.\nRequest: %s", dwl_url.c_str(), str);
		if (optVerbose) {
			string strHeaders = dump_headers(h);
			LOG_TRACE("download_file_content", "=== %s ===", "Request Headers");
			LOG_TRACE("download_file_content", "%s", strHeaders.c_str());
			COUTM("=== %s ===", "Request Headers");
			COUTM("%s", strHeaders.c_str());
		}



		httplib::Client cli(dwl_url);
		if (optVerbose) {
			cli.set_logger(
				[](const Request& req, const Response& res) {
					string l = log(req, res);
					COUTM("%s", l.c_str());
					LOG_TRACE("download_file_content", "%s", l.c_str());
				});
		}
		cli.set_connection_timeout(5, 0);
		cli.set_read_timeout(5, 0);
		cli.set_keep_alive(true);
		auto getres = cli.Get(str, h,
			[&](const char* data, size_t data_length) {
				rcvdata.append(data, data_length);
				if (optVerbose) {
					LOG_TRACE("download_file_content", "Received %d/%d bytes", data_length, rcvdata.size());
				}
				return true;
		}
			//,[&](uint64_t offset, uint64_t total_length) { COUTMS("received %d/%s", offset, total_length); return true; }
		);
		if (getres->status == 200) {
			result = true;
		}

	}
	catch (...) { LOG_ERROR("Caught Error. %s\n", ""); result = false; }
	return rcvdata;
}

std::string extract_scheme_and_host(const std::string& url) {
	std::regex url_regex(R"((https?:\/\/[^\/]+))");
	std::smatch match;
	if (std::regex_search(url, match, url_regex)) {
		return match.str(1);
	}
	return "";
}


bool download_file(const std::string& url, const std::string& output_path, bool optVerbose) {
	// Determine if the URL uses HTTPS
	bool is_https = url.substr(0, 8) == "https://";
	
	std::string scheme_host = extract_scheme_and_host(url);
	httplib::Client cli(scheme_host);

	if (is_https) {
		LOG_TRACE("utils::download_file","HTTPS, %s", scheme_host.c_str());
	}
	else {
		LOG_TRACE("utils::download_file", "HTTP, %s", scheme_host.c_str());
	}

	bool result = false;
	try {

		cli.set_connection_timeout(5, 0);
		// Enable redirection following
		cli.set_follow_location(true);
		cli.set_read_timeout(5, 0);
		cli.set_keep_alive(true);

		// Perform GET request
		auto res = cli.Get("/");
		if (optVerbose) {
			cli.set_logger(
				[](const Request& req, const Response& res) {
					string l = log(req, res);
					COUTM("%s", l.c_str());
				});
		}

		if (!res || res->status != 200) {
			LOG_ERROR("utils::download_file", "Failed to download file. HTTP Status: %s", res ? std::to_string(res->status) : "Request failed");
			return false;
		}

		// Open file for writing
		std::ofstream out_file(output_path, std::ios::binary);
		if (!out_file) {
			std::cerr << "Failed to open file for writing: " << output_path << std::endl;
			LOG_ERROR("utils::download_file", "Failed to open file for writing:  %s", output_path.c_str());

			return false;
		}

		// Write response body to file
		out_file.write(res->body.c_str(), res->body.size());
		out_file.close();
		LOG_TRACE("utils::download_file", "File downloaded successfully: %s", output_path.c_str());

		result = true;
	}
	catch (...) { COUTRS("Caught Error.\n"); result = false; }
	return result;
}



bool download_file_ext(string fullUrl, string outFile, bool optVerbose) {

	bool result = false;
	std::string scheme_host = extract_scheme_and_host(fullUrl);

	vector<string> v = split(fullUrl, '/');
	char* str = (char*)malloc(512);
	strcpy(str, "/");
	for (int i = 3; i < v.size(); i++) {
		strcat(str, v[i].c_str());
		if (i < (v.size() - 1)) { strcat(str, "/"); }
	}
	try {
		httplib::Headers h;

		h.emplace("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/99.0.4844.51 Safari/537.36");
		h.emplace("method", "GET");
		h.emplace("referer", "https://github.com/");

		LOG_TRACE("download_file", "Requesting File to server %s.\nRequest: %s", scheme_host.c_str(), str);
		if (optVerbose) {
			string strHeaders = dump_headers(h);
			COUTC("=== Request Headers ===");
			COUTC("%s", strHeaders.c_str());
		}

		string rcvdata;

		httplib::Client cli(scheme_host);
		if (optVerbose) {
			cli.set_logger(
				[](const Request& req, const Response& res) {
					string l = log(req, res);
					COUTM("%s", l.c_str());
				});
		}
		cli.set_connection_timeout(5, 0);
		// Enable redirection following
		cli.set_follow_location(true);
		cli.set_read_timeout(5, 0);
		cli.set_keep_alive(true);
		auto getres = cli.Get(str, h,
			[&](const char* data, size_t data_length) {
				rcvdata.append(data, data_length);
				if (optVerbose) {
					LOG_TRACE("download_file", "download_file", "Received %d/%d", data_length, rcvdata.size());
				}
				return true;
			}
			//,[&](uint64_t offset, uint64_t total_length) { COUTMS("received %d/%s", offset, total_length); return true; }
		);
		if (getres->status == 200) {
			LOG_TRACE("download_file", "Write %s. status %d", outFile.c_str(), getres->status);
			int written_bytes = write_content(outFile.c_str(), rcvdata.data(), rcvdata.length());
			if (written_bytes == -1) {
				throw("cannot write to file");
			}
			result = true;
		}
		else {
			COUTRS("status %d", getres->status);
		}

	}
	catch (...) { COUTRS("Caught Error.\n"); result = false; }
	return result;
}



bool get_temp_file_path(std::string &tmp_path) {
#ifdef _WIN32
	char temp_path[MAX_PATH];
	if (GetTempPathA(MAX_PATH, temp_path) == 0) {
		return false; // Failed to get temp path
	}
	char temp_file[MAX_PATH];
	if (GetTempFileNameA(temp_path, "TMP", 0, temp_file) == 0) {
		return false; // Failed to create temp file
	}
	tmp_path = std::string(temp_file);
	return true;
#else
	char temp_file[] = "/tmp/tempfileXXXXXX";
	int fd = mkstemp(temp_file);
	if (fd == -1) {
		return false; // Failed to create temp file
	}
	close(fd);
	tmp_path = std::string(temp_file);
	return true;
#endif
}


std::string generate_guid() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dist(0, 15);
	std::stringstream guid;
	const char* hex_chars = "0123456789abcdef";

	for (int i = 0; i < 36; ++i) {
		if (i == 8 || i == 13 || i == 18 || i == 23) {
			guid << "-";
		}
		else {
			guid << hex_chars[dist(gen)];
		}
	}

	return guid.str();
}



bool isNumeric(const std::string& str) {
	return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
}

std::ostringstream read_file_to_stream(const std::string& filePath) {
	std::ifstream file(filePath, std::ios::in | std::ios::binary);
	std::ostringstream content;

	if (!file) {
		std::cerr << "Error: Unable to open file: " << filePath << std::endl;
		return content; // Returns an empty stream
	}

	content << file.rdbuf();
	return content;
}

namespace Convert
{
	int wstrlen(_TCHAR* wstr)
	{
		int l_idx = 0;
		while (((char*)wstr)[l_idx] != 0) l_idx += 2;
		return l_idx;
	}

	// Allocate char string and copy TCHAR->char->string 

	char* wstrdup(_TCHAR* wSrc)
	{
		int l_idx = 0;
		int l_len = wstrlen(wSrc);
		char* l_nstr = (char*)malloc(l_len);
		if (l_nstr)
		{
			do
			{
				l_nstr[l_idx] = (char)wSrc[l_idx];
				l_idx++;
			} while ((char)wSrc[l_idx] != 0);
		}
		l_nstr[l_idx] = 0;
		return l_nstr;
	}

	// allocate argn structure parallel to argv 
	// argn must be released 

	char** allocate_argn(int argc, _TCHAR* argv[])
	{
		char** l_argn = (char**)malloc(argc * sizeof(char*));
		for (int idx = 0; idx < argc; idx++)
		{
			l_argn[idx] = wstrdup(argv[idx]);
		}

		return l_argn;
	}

	// release argn and its content 

	void release_argn(int argc, char** nargv)
	{
		for (int idx = 0; idx < argc; idx++)
		{
			free(nargv[idx]);
		}
		free(nargv);
	}

	void __TrimDecimalString(PWCHAR str)
	{
		int length = lstrlenW(str);
		for (; str[length - 1] == L'0'; length--)
		{
			str[length - 1] = L'\0';
		}

		if (str[length - 1] == L'.') str[length - 1] = L'\0';
	}

	LPWSTR StringToString(LPCSTR str)
	{
		int size = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
		PWCHAR result = new WCHAR[size];
		MultiByteToWideChar(CP_UTF8, 0, str, -1, result, size);

		return result;
	}
	LPSTR StringToString(LPCWSTR str)
	{
		int size = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
		PCHAR result = new CHAR[size];
		WideCharToMultiByte(CP_UTF8, 0, str, -1, result, size, NULL, NULL);

		return result;
	}

	LPWSTR Int32ToString(__int32 value, int base)
	{
		WCHAR buffer[20];
		_ltow_s(value, buffer, base);

		PWCHAR result = new WCHAR[lstrlenW(buffer) + 1];
		StrCpyW(result, buffer);

		return result;
	}
	LPWSTR UInt32ToString(unsigned __int32 value, int base)
	{
		WCHAR buffer[20];
		_ultow_s(value, buffer, base);

		PWCHAR result = new WCHAR[lstrlenW(buffer) + 1];
		StrCpyW(result, buffer);

		return result;
	}
	LPWSTR Int64ToString(__int64 value, int base)
	{
		WCHAR buffer[30];
		_i64tow_s(value, buffer, sizeof(buffer) / sizeof(WCHAR), base);

		PWCHAR result = new WCHAR[lstrlenW(buffer) + 1];
		StrCpyW(result, buffer);

		return result;
	}
	LPWSTR UInt64ToString(unsigned __int64 value, int base)
	{
		WCHAR buffer[30];
		_ui64tow_s(value, buffer, sizeof(buffer) / sizeof(WCHAR), base);

		PWCHAR result = new WCHAR[lstrlenW(buffer) + 1];
		StrCpyW(result, buffer);

		return result;
	}
	LPWSTR FloatToString(float value)
	{
		WCHAR buffer[50];
		swprintf_s(buffer, L"%f", value);

		__TrimDecimalString(buffer);
		PWCHAR result = new WCHAR[lstrlenW(buffer) + 1];
		StrCpyW(result, buffer);

		return result;
	}
	LPWSTR DoubleToString(double value)
	{
		WCHAR buffer[50];
		swprintf_s(buffer, L"%.20f", value);

		__TrimDecimalString(buffer);
		PWCHAR result = new WCHAR[lstrlenW(buffer) + 1];
		StrCpyW(result, buffer);

		return result;
	}

	__int32 StringToInt32(LPCWSTR str, int base)
	{
		return wcstol(str, NULL, base);
	}
	unsigned __int32 StringToUInt32(LPCWSTR str, int base)
	{
		return wcstoul(str, NULL, base);
	}
	__int64 StringToInt64(LPCWSTR str, int base)
	{
		return wcstoll(str, NULL, base);
	}
	unsigned __int64 StringToUInt64(LPCWSTR str, int base)
	{
		return wcstoull(str, NULL, base);
	}
	float StringToFloat(LPCWSTR str)
	{
		return wcstof(str, NULL);
	}
	double StringToDouble(LPCWSTR str)
	{
		return wcstod(str, NULL);
	}

	LPWSTR UInt32ToHexString(unsigned __int32 value)
	{
		LPWSTR buffer = UInt32ToString(value, 16);
		PWCHAR result = new WCHAR[11];

		StrCpyW(result, L"0x00000000");
		StrCpyW(&result[10 - lstrlenW(buffer)], buffer);
		delete[] buffer;

		return result;
	}
	LPWSTR UInt64ToHexString(unsigned __int64 value)
	{
		LPWSTR buffer = UInt64ToString(value, 16);
		PWCHAR result = new WCHAR[19];

		StrCpyW(result, L"0x0000000000000000");
		StrCpyW(&result[18 - lstrlenW(buffer)], buffer);
		delete[] buffer;

		return result;
	}
	LPWSTR BytesToHexView(LPBYTE data, DWORD length)
	{
		PWCHAR result = new WCHAR[((length - 1) / 16 + 1) * 79 + 1];

		for (DWORD i = 0, offset = 0; i < length; i += 16)
		{
			LPWSTR line = UInt32ToString(i, 16);
			StrCpyW(&result[offset], L"00000000");
			StrCpyW(&result[offset + 8 - lstrlenW(line)], line);
			StrCpyW(&result[offset + 8], L"h: ");

			delete[] line;
			offset += 11;

			for (DWORD j = 0; j < 16; j++)
			{
				if (i + j < length)
				{
					LPWSTR number = UInt32ToString(data[i + j], 16);
					StrCpyW(&result[offset], L"00");
					StrCpyW(&result[offset + 2 - lstrlenW(number)], number);
					result[offset + 2] = L' ';

					delete[] number;
				}
				else
				{
					result[offset] = L' ';
					result[offset + 1] = L' ';
					result[offset + 2] = L' ';
				}

				offset += 3;
			}

			result[offset++] = L';';
			result[offset++] = L' ';

			for (DWORD j = 0; j < 16; j++)
			{
				if (i + j < length)
				{
					result[offset++] = data[i + j] >= 32 && data[i + j] <= 126 ? (WCHAR)data[i + j] : L'.';
				}
				else
				{
					result[offset++] = L' ';
				}
			}

			StrCpyW(&result[offset], L"\r\n");
			offset += 2;
		}

		return result;
	}
}

