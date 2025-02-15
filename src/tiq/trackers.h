
//==============================================================================
//
// trackers.h : tracker-related  code
//
//==============================================================================
//  Copyright (C) Guilaume Plante 2020 <cybercastor@icloud.com>
//==============================================================================


#ifndef __TRACKERS_H__
#define __TRACKERS_H__


#include <iostream>
#include <fstream>
#include "httplib.h"
#include <regex>
#include <vector>

#include <string>

bool extract_trackers_from_file(const std::string& file_path, std::vector<std::string>& tracker_list);
bool test_trackers_list_out(std::vector<std::string> tracker_list, bool json);
bool test_trackers_list_res(std::vector<std::string> tracker_list, class TrackerTestResults& test_results);
bool test_single_tracker_out(const std::string& tracker_url, bool is_json, int timeout_sec = 5);
bool test_single_tracker_res(const std::string& tracker_url, class TrackerTestResults& test_results, int timeout_sec = 5);
void rate_indexer(const std::string& magnet_uri, bool is_json, int timeout_sec = 5);
void rank_trackers(TrackerTestResults& test_results);
double normalize(double value, double minVal, double maxVal);
#endif//__TRACKERS_H__