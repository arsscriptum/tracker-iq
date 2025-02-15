
//==============================================================================
//
// trackers.cpp : tracker-related  code
//
//==============================================================================
//  Copyright (C) Guilaume Plante 2020 <cybercastor@icloud.com>
//==============================================================================


#include "stdafx.h"
#include "trackers.h"
#include <Windows.h>
#include <system_error>
#include <memory>
#include <string>
#include "log.h"
#include "utils.h"
#include "httplib.h"
#include "test_results.h"

#include "Shlwapi.h"
#include "config.h"
#include "log.h"
#include <codecvt>
#include <locale>

#include <unordered_map>
#include <iterator>
#include <regex>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <cstdlib>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif


#include <libtorrent/settings_pack.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/error_code.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/session_params.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_status.hpp>
#include <libtorrent/error_code.hpp>


extern unsigned  int g_default_timeout_sec;
extern unsigned  int g_default_num_want;
using namespace httplib;
using namespace std;

int max_url_len = 48;
int unique_id = 0;

bool extract_trackers_from_file(const std::string& file_path, std::vector<std::string> &tracker_list) {
    bool result = false;
    std::ifstream file(file_path);
    if (!file) {
        LOG_ERROR("trackers::extract_trackers_from_file","failed to open file %s", file_path.c_str())
        return false;
    }
	tracker_list.clear();
    std::string line;
    std::regex tracker_regex(R"((udp|https?|wss):\/\/[^\s]+\/announce)");
    while (std::getline(file, line)) {
        if (!line.empty()) {
            std::smatch match;
            if (std::regex_search(line, match, tracker_regex)) {
                std::string tmp_tracker = match.str(0);
                result = true;
                LOG_DEBUG("trackers::extract_trackers_from_file", "found %s", tmp_tracker.c_str())
				tracker_list.push_back(tmp_tracker);
            }else{
               LOG_DEBUG("trackers::extract_trackers_from_file", "no match for %s", line.c_str())
            }
        }
    }
    return result;
}



bool test_trackers_list_out(std::vector<std::string> tracker_list, bool json) {

	if (json) {
		std::cout << "[" << std::endl << std::flush;
	}
	else {
		std::cout << std::endl << std::endl << std::left << std::setw(max_url_len) << "Tracker Address"
			<< std::setw(10) << "Status"
			<< std::setw(8) << "Peers"
			<< std::setw(12) << "Latency"
			<< "Details" << std::endl;
		std::cout << std::endl << std::flush;
	}
	for (auto&& tracker_url : tracker_list) {
		LOG_DEBUG("trackers::test_trackers_list_out", "check %d", tracker_url.c_str());


		bool first = true;
		
		if (tracker_url.empty()) {
			continue;
		}

		if (!first) {
			if (json) {
				std::cout << "," << std::endl;
			}
		}else {
			first = false;
		}

		test_single_tracker_out(tracker_url, json, g_default_timeout_sec);

	}
		
	if (json) {
		std::cout << std::endl << "]" << std::endl << std::flush;
	}

	

	return  true;
}



bool test_trackers_list_res(std::vector<std::string> tracker_list, TrackerTestResults &test_results) {

	DEJA_BOOKMARK("trackers::test_trackers_list_res", "test_trackers_list_res");
	LOG_DEBUG("trackers::test_trackers_list","test_trackers_list");
	for (auto&& tracker_url : tracker_list) {
		LOG_DEBUG("trackers::test_trackers_list_res", "check %d", tracker_url.c_str());
		test_single_tracker_res(tracker_url, test_results, g_default_timeout_sec);
	}

	if (test_results.Count() == 0) {
		return false;
	}

	LOG_DEBUG("trackers::test_trackers_list_res", "num results: %d", test_results.Count());

	return  true;
}


bool test_single_tracker_out(const std::string& tracker_url, bool is_json, int timeout_sec) {
	LOG_DEBUG("trackers::test_single_tracker_out", "start tracker test: %s. timeout_sec %d, num_want %d", tracker_url.c_str(), timeout_sec, g_default_num_want);
	DEJA_BOOKMARK("trackers::test_single_tracker_out", "test_single_tracker_out");
	std::string tmp_path = "";
	get_temp_file_path(tmp_path);

	lt::settings_pack settings;
	
	settings.set_int(lt::settings_pack::alert_mask, lt::alert_category::tracker);
	settings.set_int(lt::settings_pack::num_want, g_default_num_want);

	lt::session session(settings);

	lt::add_torrent_params params;
	params.trackers.push_back(tracker_url);
	params.info_hashes.v1 = lt::sha1_hash("01234567890123456789"); // Fake info-hash for testing
	params.name = generate_guid();

	params.save_path = tmp_path;

	auto handle = session.add_torrent(std::move(params));

	bool tracker_responded = false;
	int num_peers = 0;
	std::string details = "n/a";

	auto start_time = std::chrono::high_resolution_clock::now();
	int iterations = timeout_sec * 2;

	//LOG_DEBUG("trackers::test_single_tracker_out", "start tracker test: %s. version %d", tracker_url.c_str(), params.version, params.trackerid, params.peers, params.url.c_str(), params.info_hash.to_string().c_str(), params.active_time, params.name.c_str(), (int)params.flags);
	//LOG_DEBUG("trackers::test_single_tracker_out", "trackerid %d, peers %d, url %s, hash %s,  active_time %d, name %s. flags %d",  params.trackerid, params.peers, params.url.c_str(), params.info_hash.to_string().c_str(), params.active_time, params.name.c_str(), (int)params.flags);

	for (int i = 0; i < iterations; ++i) { // 10 seconds timeout
		std::vector<lt::alert*> alerts;
		session.pop_alerts(&alerts);
		//LOG_DEBUG("trackers::test_single_tracker_out", "iteration: %d", i);

		for (const auto* alert : alerts) {
			//LOG_DEBUG("trackers::test_single_tracker_out", "iteration: %d", i);
			if (auto reply = lt::alert_cast<lt::tracker_reply_alert>(alert)) {
				tracker_responded = true;
				num_peers = reply->num_peers;
				unsigned int version = (unsigned int)reply->version;
				details = reply->message();
				//LOG_DEBUG("trackers::test_single_tracker_out", "num_peers: %d, version: %d, details: %s", num_peers, version,details.c_str());
			}
			else if (auto error = lt::alert_cast<lt::tracker_error_alert>(alert)) {
				details = error->error.message();
				//tracker_responded = false;
				//LOG_DEBUG("trackers::test_single_tracker_out", "error %s", details.c_str());
			}
		}

		if (tracker_responded) break;

		std::this_thread::sleep_for(0.5s);
	}

	auto end_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> response_time = end_time - start_time;
	double millis = response_time.count() * 1000.0;

	// Get current timestamp
	auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::ostringstream timestamp;
	timestamp << std::put_time(std::localtime(&now), "%Y-%m-%dT%H:%M:%S");
	bool is_tracker_valid = (tracker_responded == true) && (num_peers > 0);

	LOG_DEBUG("trackers::test_single_tracker_out", "[%s] %d peers, ping %d, on: %s", is_tracker_valid ? "valid" : "no response", num_peers, millis, timestamp.str().c_str());

	/*
	TODO:  return this instead
	{
	  "unique_id": 1,
	  "is_valid": true,
	  "num_peers": 3,
	  "response_time": 2.33,
	  "url": "udp://tracker.dler.org:6969/announce",
	  "last_tested": "2025-02-07T09:12:42"
	}
	*/
	float rank_score = 0.f;
	if (is_json) {
		// Manually create a JSON string using JSMN-style formatting
		std::ostringstream json_output;
		json_output << "{"
			<< "\"unique_id\": " << unique_id++ << ", "
			<< "\"is_valid\": " << (is_tracker_valid ? "true" : "false") << ", "
			<< "\"num_peers\": " << num_peers << ", "
			<< "\"response_time\": " << millis << ", "
			<< "\"url\": \"" << tracker_url << "\", "
			<< "\"last_tested\": \"" << timestamp.str() << "\""
			<< "\"rank_score\": " << std::fixed << std::setprecision(4) << rank_score << ", "
			<< "}" << std::flush;

		std::cout << json_output.str();
	}
	else {
		std::cout << std::left << std::setw(max_url_len) << tracker_url
			<< std::setw(10) << (is_tracker_valid ? "valid" : "invalid")
			<< std::setw(8) << (tracker_responded ? std::to_string(num_peers) : "0")
			<< std::setw(12) << (tracker_responded ? std::to_string(millis) : "n/a")
			<< details << std::endl << std::flush;
	}

	session.remove_torrent(handle);
	return  true;
}



bool test_single_tracker_res(const std::string& tracker_url, TrackerTestResults& test_results, int timeout_sec) {
	std::string tmp_path = "";
	DEJA_BOOKMARK("trackers::test_single_tracker_res", "test_single_tracker_res");
	get_temp_file_path(tmp_path);
	LOG_DEBUG("trackers::test_single_tracker_res", "start tracker test: %s. timeout_sec %d, num_want %d", tracker_url.c_str(), timeout_sec, g_default_num_want);
	lt::settings_pack settings;
	settings.set_int(lt::settings_pack::alert_mask, lt::alert_category::tracker);
	settings.set_int(lt::settings_pack::num_want, g_default_num_want);

	lt::session session(settings);

	lt::add_torrent_params params;
	params.trackers.push_back(tracker_url);
	params.info_hashes.v1 = lt::sha1_hash("01234567890123456789"); // Fake info-hash for testing
	params.name = generate_guid();

	params.save_path = tmp_path;

	auto handle = session.add_torrent(std::move(params));

	bool tracker_responded = false;
	int num_peers = 0;
	std::string details = "n/a";

	auto start_time = std::chrono::high_resolution_clock::now();
	int iterations = timeout_sec * 2;

	//LOG_DEBUG("trackers::test_single_tracker_out", "start tracker test: %s. version %d", tracker_url.c_str(), params.version, params.trackerid, params.peers, params.url.c_str(), params.info_hash.to_string().c_str(), params.active_time, params.name.c_str(), (int)params.flags);
	//LOG_DEBUG("trackers::test_single_tracker_out", "trackerid %d, peers %d, url %s, hash %s,  active_time %d, name %s. flags %d", params.trackerid, params.peers, params.url.c_str(), params.info_hash.to_string().c_str(), params.active_time, params.name.c_str(), (int)params.flags);

	for (int i = 0; i < iterations; ++i) { // 10 seconds timeout
		std::vector<lt::alert*> alerts;
		session.pop_alerts(&alerts);
		//LOG_DEBUG("trackers::test_single_tracker_res", "iteration: %d", i);
		for (const auto* alert : alerts) {
			if (auto reply = lt::alert_cast<lt::tracker_reply_alert>(alert)) {
				tracker_responded = true;
				num_peers = reply->num_peers;
				unsigned int version = (unsigned int)reply->version;
				details = reply->message();
				//LOG_DEBUG("trackers::test_single_tracker_out::listening", "num_peers: %d, version: %d, details: %s", num_peers, version, details.c_str());
			}
			else if (auto error = lt::alert_cast<lt::tracker_error_alert>(alert)) {
				details = error->error.message();
				//tracker_responded = false;
				//LOG_DEBUG("trackers::test_single_tracker_res::listening", "error %s", details.c_str());
			}
		}

		if (tracker_responded) break;

		std::this_thread::sleep_for(0.5s);
	}

	auto end_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> response_time = end_time - start_time;
	double millis = response_time.count() * 1000.0;
	bool is_tracker_valid = (tracker_responded == true) && (num_peers > 0);
	// Get current timestamp
	auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::ostringstream timestamp;
	timestamp << std::put_time(std::localtime(&now), "%Y-%m-%dT%H:%M:%S");
	
	LOG_DEBUG("trackers::test_single_tracker_res", "[%s] %d peers, ping %d, on: %s", is_tracker_valid ? "valid" : "no response", num_peers, millis, timestamp.str().c_str());
	test_results.addResult(unique_id++, is_tracker_valid, num_peers, millis, tracker_url, timestamp.str());

	session.remove_torrent(handle);

	return  true;
}



void rate_indexer(const std::string& magnet_uri, bool is_json, int timeout_sec) {
	// Create session parameters and a session
	std::string tmp_path = "";
	get_temp_file_path(tmp_path);

	lt::settings_pack settings;
	settings.set_int(lt::settings_pack::alert_mask, lt::alert::tracker_notification);
	settings.set_int(lt::settings_pack::num_want, g_default_num_want);
	lt::session session(settings);
	bool tracker_responded = false;
	// Add a magnet URI to the session
	lt::error_code ec;
	lt::add_torrent_params params = lt::parse_magnet_uri(magnet_uri, ec);

	if (ec) {
		std::cerr << "Failed to parse magnet URI: " << ec.message() << std::endl;
		return;
	}

	params.save_path = tmp_path; // Path where files will be downloaded
	auto handle = session.add_torrent(std::move(params));

	LOG_DEBUG("trackers::rate_indexer","Connecting to trackers...");

	// Trackers and quality rating
	std::map<std::string, double> tracker_ratings;
	std::map<std::string, int> tracker_ratings_peers;

	auto start_time = std::chrono::high_resolution_clock::now();
	int iterations = timeout_sec * 2;
	for (int i = 0; i < iterations; ++i) {
		std::vector<lt::alert*> alerts;
		session.pop_alerts(&alerts);

		for (auto alert : alerts) {
			if (auto t_alert = lt::alert_cast<lt::tracker_reply_alert>(alert)) {
				auto tracker = t_alert->url;
				tracker_responded = true;

				auto end_time = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double> response_time = end_time - start_time;
				tracker_ratings[tracker] += (t_alert->num_peers * response_time.count());
				tracker_ratings_peers[tracker] += (t_alert->num_peers);
				LOG_DEBUG("trackers::rate_indexer", "tracker %s, responded with %d peers in %s seconds", tracker.c_str(), t_alert->num_peers, std::to_string(response_time.count()).c_str());
				std::cout << "Tracker: " << tracker << " responded with " << t_alert->num_peers << " peers in " << std::to_string(response_time.count()) + " seconds" << std::endl;
			}
		}

		std::this_thread::sleep_for(0.5s);
	}



	std::cout << "\n--- Tracker Quality Ratings Peers ---\n";
	for (const auto& [tracker, rating] : tracker_ratings_peers) {
		std::cout << "Tracker: " << tracker << ", Quality Rating: " << rating << std::endl;
	}
	std::cout << "\n--- Tracker Quality Ratings Calibrated with Time ---\n";
	for (const auto& [tracker, rating] : tracker_ratings_peers) {
		std::cout << "Tracker: " << tracker << ", Quality Rating: " << rating << std::endl;
	}
	session.remove_torrent(handle);
}

// Function to normalize a value (min-max scaling)
double normalize(double value, double minVal, double maxVal) {
	return (maxVal == minVal) ? 0.0 : (value - minVal) / (maxVal - minVal);
}


// Function to rank trackers based on response time and number of peers
void rank_trackers(TrackerTestResults& test_results) {
	LOG_PROFILE("trackers::ranking::rank_trackers");
	DEJA_BOOKMARK("trackers::ranking::rank_trackers","RANKING");
	LOG_DEBUG("trackers::ranking::rank_trackers", "number of results %d", test_results.Count());
	if (test_results.empty()) {
		LOG_DEBUG("trackers::ranking::rank_trackers", "EMPTY");
	}
	LOG_DEBUG("trackers::ranking::rank_trackers", "number of results %d", test_results.Count());

	DEJA_CONSOLE_WRITE("trackers::ranking::rank_trackers");
	std::vector<TrackerTest> trackers = test_results.get_results();

	// Find min and max for normalization
	double minPeers = test_results.get_min_peers();
	double maxPeers = test_results.get_max_peers();
	double minResponse = test_results.get_min_response_time();
	double maxResponse = test_results.get_max_response_time();
	LOG_DEBUG("trackers::ranking::rank_trackers", "minPeers %d", minPeers);
	LOG_DEBUG("trackers::ranking::rank_trackers", "minResponse %.4f", minResponse);
	LOG_DEBUG("trackers::ranking::rank_trackers", "maxPeers %d", maxPeers);
	LOG_DEBUG("trackers::ranking::rank_trackers", "maxResponse %.4f", maxResponse);

	test_results.sanitize_results();

	// Compute ranking score for each tracker
	for (std::vector<TrackerTest>::iterator it = trackers.begin(); it != trackers.end(); ++it) {
		if (!it->get_is_valid()) {
			continue;
		}
		TrackerTest* tracker = &(*it);

		float cfg_weight_ping = CONFIG.getWeightPing();
		float cfg_weight_peers = CONFIG.getWeightPeers();
		if ((cfg_weight_peers + cfg_weight_ping) != 1.0f) {
			app_error("invalid configuration values for weight_peers/weight_ping (total should be 1.0)");
			throw("invalid configuration values for weight_peers/weight_ping (total should be 1.0)");
		}
		double normalizedPeers = normalize(tracker->get_num_peers(), minPeers, maxPeers);
		double normalizedResponse = normalize(tracker->get_response_time(), minResponse, maxResponse);
		LOG_DEBUG("trackers::ranking::rank_trackers", "cfg_weight_ping %.2f cfg_weight_peers %.2f", cfg_weight_ping, cfg_weight_peers);
		
		LOG_DEBUG("trackers::ranking::rank_trackers", "normalizedPeers %.6f", normalizedPeers);
		LOG_DEBUG("trackers::ranking::rank_trackers", "normalizedResponse %.6f", normalizedResponse);
		double a1 = (cfg_weight_peers * normalizedPeers);
		double a2 = (1.0f - normalizedResponse);
		double a3 = (cfg_weight_ping * a2);
		double rank = a1 + a3;
		LOG_DEBUG("trackers::ranking::rank_trackers", "cfg_weight_peers * normalizedPeers %.6f", a1);
		LOG_DEBUG("trackers::ranking::rank_trackers", "1.0f - normalizedResponse          %.6f", a2);
		LOG_DEBUG("trackers::ranking::rank_trackers", "cfg_weight_ping * a2               %.6f", a3);
		LOG_DEBUG("trackers::ranking::rank_trackers", "a1 + a3                            %.6f", rank);
		LOG_WARNING("RANKING","SET VALUE TO %.5f", rank);
		tracker->set_ranking(rank); // Inverted response time
		LOG_WARNING("RANKING", "CONFIRM %.5f", tracker->get_ranking());
		LOG_DEBUG("trackers::ranking::rank_trackers", "[%d] rank %.4f", tracker->get_unique_id(), tracker->get_ranking());
		
	}
	
	

	Sleep(1000);
	// Sort trackers in descending order of rankScore
	std::sort(trackers.begin(), trackers.end(), []( TrackerTest& a,  TrackerTest& b) {
		return a.get_ranking() > b.get_ranking();
		});
	Sleep(3000);
}
