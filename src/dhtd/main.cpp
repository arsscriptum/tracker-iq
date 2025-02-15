
//==============================================================================
//
//     main.cpp
//
//============================================================================
// Copyright (C)  Guillaume Plante <codegp@icloud.com>
//==============================================================================



#include "stdafx.h"
#include "cmdline.h"
#include "Shlwapi.h"
#include "log.h"

#include <codecvt>
#include <locale>
#include <vector>
#include <unordered_map>
#include <iterator>
#include <regex>
#include <filesystem>
#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <fstream>
#include "Shlwapi.h"
#include "log.h"
#include <ctime>
#include <iostream>
#include <string>
#include <algorithm>

#include "inireader.h"
#include "config.h"
#include "version.h"
#include "DejaLib.h"

#include <libtorrent/settings_pack.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/error_code.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/session_params.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_status.hpp>
#include <libtorrent/error_code.hpp>

#include <boost/asio/ip/address.hpp>


#include "DejaLib.h"

bool g_is_verbose = false;
unsigned  int g_default_num_want = 1;
using namespace std;
using namespace std::chrono_literals;

#pragma message( "Compiling " __FILE__ )
#pragma message( "Last modified on " __TIMESTAMP__ )


// Struct to store DHT Reply Data
struct DHTReplyData {
	std::string url;
	int num_peers;
};

// Global storage for DHT replies
std::map<int, DHTReplyData> dht_replies;
int dht_reply_id = 0;  // Auto-incrementing ID


bool pre_process_alert(const lt::alert* a);
void process_alerts(lt::session& s);
int get_dht_reply_id_by_url(const std::string& url);
std::string get_dht_reply_url_by_id(int id);

int log_config_values() {

	try {
		Config& config = Config::getInstance();
		CONFIG_LOG("ipv6_enabled:        %s", CONFIG.net_ipv6_enabled() ? "true" : "false");
		CONFIG_LOG("enable_incoming_utp: %s", CONFIG.net_incoming_utp() ? "true" : "false");
		CONFIG_LOG("enable_outgoing_utp: %s", CONFIG.net_outgoing_utp() ? "true" : "false");
		CONFIG_LOG("Listen interface   \"%s\"", CONFIG.net_listen_ifaces().c_str());
		CONFIG_LOG("Listen interface   \"%s\"", CONFIG.net_outgoing_ifaces().c_str());
		CONFIG_LOG("bootstrap_nodes:   \"%s\"", CONFIG.net_bootstrap_nodes().c_str());

		CONFIG_LOG("Console Logging: %s", config.isConsoleEnabled() ? "Enabled" : "Disabled");
		CONFIG_LOG("Log File: %s", config.getLogFile().c_str());

		CONFIG_LOG("Debug Mode: %s", config.isDebugEnabled() ? "Enabled" : "Disabled");
		CONFIG_LOG("Debug Pause: %d seconds", config.getDebugPause());
		CONFIG_LOG("Exit Pause: %d seconds", config.getExitPause());
	}
	catch (const std::exception& e) {
		LOG_TRACE("main", "Exception occurred: %s", e.what());
	}

	return 0;
}

int main(int argc, TCHAR** argv, TCHAR envp)
{

#ifdef UNICODE
	char** argn = (char**)Convert::allocate_argn(argc, argv);
#else
	char** argn = argv;
#endif // UNICODE

	CmdLineUtil::getInstance()->initializeCmdlineParser(argc, argn);

	CmdlineParser* inputParser = CmdLineUtil::getInstance()->getInputParser();

	CmdlineOption cmdlineOptionHelp({		"-h", "--help" },     "display this help");
	CmdlineOption cmdlineOptionVerbose({	"-v", "--verbose" },  "verbose output");
	CmdlineOption cmdlineOptionNoBanner({ "-n", "--nobanner" }, "no banner");
	CmdlineOption cmdlineOptionQuiet({ "-q", "--quiet" }, "quiet");
	CmdlineOption cmdlineOptionConfigFile({ "-c", "--config" }, "config gile");

	inputParser->addOption(cmdlineOptionHelp);
	inputParser->addOption(cmdlineOptionVerbose);
	inputParser->addOption(cmdlineOptionQuiet);
	inputParser->addOption(cmdlineOptionConfigFile);
	inputParser->addOption(cmdlineOptionNoBanner);

	bool optHelp = inputParser->isSet(cmdlineOptionHelp);
	bool optVerbose = inputParser->isSet(cmdlineOptionVerbose);
	bool optNoBanner = inputParser->isSet(cmdlineOptionNoBanner);
	bool optQuiet = inputParser->isSet(cmdlineOptionQuiet);
	bool optConfig = inputParser->isSet(cmdlineOptionConfigFile);

	g_is_verbose = optVerbose;
	if (optQuiet) {
		g_is_verbose = false;
	}
	
	if (optQuiet && optVerbose) {
		COUTCS("Warning: Quiet and Verbose: Verbose superceed Quiet...");
	}

	if (optNoBanner == false) {
		banner();
	}
	if (optHelp) {
		usage();
		return 0;
	}
	std::string file_path = "";
	std::string tracker_url = "";
	
	if (optConfig) {

		std::string tmp_config_file = "";
		if (inputParser->get_option_argument(cmdlineOptionConfigFile, tmp_config_file)) {
			LOG_TRACE("main::arguments::optTracker", "tmp_config_file %s", tmp_config_file.c_str());

			bool is_valid = Config::getInstance().ValidateConfigChecksum(tmp_config_file);
			if (is_valid) {
				LOG_TRACE("main::ValidateConfigChecksum", "tmp_config_file %s VALID", tmp_config_file.c_str());
			}else {
				LOG_WARNING("main::ValidateConfigChecksum", "tmp_config_file %s NOT VALID", tmp_config_file.c_str());
#ifdef _RELEASE 
				throw("Error: Invalid config file");
				return -3;
#endif 
			}

			Config::getInstance().Initialize(tmp_config_file);
		}
		else {
			LOG_TRACE("main::arguments::optTracker", "missing url");
			usage_error("missing config Path");
			return -1;
		}
	}
	else {
		Config::getInstance().Initialize();
	}

	log_config_values();

	INFOLOG("=======================================");
	NOTICELOG("libtorrent settings");
	INFOLOG("ipv6_enabled:        %s", CONFIG.net_ipv6_enabled() ?"true":"false");
	INFOLOG("enable_incoming_utp: %s", CONFIG.net_incoming_utp() ? "true" : "false");
	INFOLOG("enable_outgoing_utp: %s", CONFIG.net_outgoing_utp() ? "true" : "false");
	INFOLOG("Listen interface   \"%s\"", CONFIG.net_listen_ifaces().c_str());
	INFOLOG("Listen interface   \"%s\"", CONFIG.net_outgoing_ifaces().c_str());
	INFOLOG("bootstrap_nodes:   \"%s\"", CONFIG.net_bootstrap_nodes().c_str());
	INFOLOG("=======================================");


	lt::settings_pack settings;
	//settings.set_bool(lt::settings_pack::enable_ipv6, false);
	//settings.set_int(lt::settings_pack::udp_port, 6881);  // Change to an available port
	settings.set_bool(lt::settings_pack::enable_outgoing_utp, CONFIG.net_outgoing_utp());
	settings.set_bool(lt::settings_pack::enable_incoming_utp, CONFIG.net_incoming_utp());
	settings.set_str(lt::settings_pack::listen_interfaces, "10.0.0.138:6881");
	//settings.set_str(lt::settings_pack::listen_interfaces, CONFIG.net_listen_ifaces());

	// Correctly formatted bootstrap nodes (no spaces after commas)

	// Enable DHT before setting bootstrap nodes
	settings.set_bool(lt::settings_pack::enable_dht, true);

	settings.set_str(lt::settings_pack::dht_bootstrap_nodes, "router.bittorrent.com:6881,router.utorrent.com:6881,dht.transmissionbt.com:6881");
	//settings.set_str(lt::settings_pack::dht_bootstrap_nodes, CONFIG.net_bootstrap_nodes());
	lt::session s(settings);

	// Start processing DHT alerts
	process_alerts(s);

	if (CONFIG.isDebugEnabled()) {
		DEJA_CONSOLE_WRITE("DO YOU WANT TO SLEEP ?");
		char buf[256];
		DEJA_CONSOLE_READ(buf, 255);
		if (!strcmp(buf, "yes")) {
			DEJA_CONSOLE_ECHO("OK");
		}
		LOG_DEBUG("main", "Sleeping for %d", CONFIG.getExitPause());
		Sleep(CONFIG.getExitPause());
	}
	
	return 0;
}

bool pre_process_alert(const lt::alert* a) {
	bool res = true;
	switch (a->type())
	{
	case lt::dht_sample_infohashes_alert::alert_type:
	{
		LOG_DEBUG("dhtd::process_alerts", "received alert type dht_sample_infohashes_alert (%d)", lt::dht_sample_infohashes_alert::alert_type);
	}
	break;

	case lt::dht_outgoing_get_peers_alert::alert_type:
	{
		LOG_DEBUG("dhtd::process_alerts", "received alert type dht_outgoing_get_peers_alert (%d)", lt::dht_outgoing_get_peers_alert::alert_type);
	}
	break;

	case lt::dht_bootstrap_alert::alert_type:
	{
		LOG_DEBUG("dhtd::process_alerts", "received alert type dht_bootstrap_alert (%d)", lt::dht_bootstrap_alert::alert_type);
	}
	break;

	case lt::dht_reply_alert::alert_type:
	{
		LOG_DEBUG("dhtd::process_alerts", "received alert type dht_reply_alert (%d)", lt::dht_reply_alert::alert_type);
	}
	break;

	case lt::dht_announce_alert::alert_type:
	{
		LOG_DEBUG("dhtd::process_alerts", "received alert type dht_announce_alert (%d)", lt::dht_announce_alert::alert_type);
	}
	break;

	case lt::dht_get_peers_alert::alert_type:
	{
		LOG_DEBUG("dhtd::process_alerts", "received alert type dht_get_peers_alert (%d)", lt::dht_get_peers_alert::alert_type);
	}
	break;

	case lt::dht_error_alert::alert_type:
	{
		LOG_DEBUG("dhtd::process_alerts", "received alert type dht_error_alert (%d)", lt::dht_error_alert::alert_type);
		auto dht_e = lt::alert_cast<lt::dht_error_alert>(a);
		ERROR_DESC("dhtd::process_alerts", "operation id %d. %s %s", (unsigned int)dht_e->operation, dht_e->error.message().c_str(), dht_e->error.to_string().c_str());
		res = false;
	}
	break;
	case lt::portmap_error_alert::alert_type:
	{
		LOG_DEBUG("dhtd::process_alerts", "received alert type portmap_error_alert (%d)", lt::portmap_error_alert::alert_type);
		auto dht_e = lt::alert_cast<lt::portmap_error_alert>(a);
		ERROR_DESC("dhtd::process_alerts", "what ? \"%s\" . %s %s. transport %d. mapping %d.", dht_e->what(), dht_e->error.message().c_str(), dht_e->error.to_string().c_str(), dht_e->local_address.to_string().c_str(), (unsigned int)dht_e->map_transport, (int)dht_e->mapping);
		res = false;
	}
	break;
	
	case lt::udp_error_alert::alert_type:
	{
		LOG_DEBUG("dhtd::process_alerts", "received alert type udp_error_alert (%d)", lt::udp_error_alert::alert_type);
		if (auto* udp_e = libtorrent::alert_cast<libtorrent::udp_error_alert>(a)) {
			ERROR_DESC("dhtd::process_alerts",
				"UDP error: \"%s\" . %s %s. Endpoint: %s.",
				udp_e->what(),
				udp_e->error.message().c_str(),
				udp_e->error.to_string().c_str(),
				udp_e->endpoint.address().to_string().c_str());
		}
		res = false;
	}
	break;
	case lt::dht_immutable_item_alert::alert_type:
	{
		LOG_DEBUG("dhtd::process_alerts", "received alert type dht_immutable_item_alert (%d)", lt::dht_immutable_item_alert::alert_type);
	}
	break;

	case lt::dht_mutable_item_alert::alert_type:
	{
		LOG_DEBUG("dhtd::process_alerts", "received alert type dht_mutable_item_alert (%d)", lt::dht_mutable_item_alert::alert_type);
	}
	break;

	case lt::dht_put_alert::alert_type:
	{
		LOG_DEBUG("dhtd::process_alerts", "received alert type dht_put_alert (%d)", lt::dht_put_alert::alert_type);
	}
	break;

	case lt::dht_stats_alert::alert_type:
	{
		LOG_DEBUG("dhtd::process_alerts", "received alert type dht_stats_alert (%d)", lt::dht_stats_alert::alert_type);
	}
	break;

	case lt::dht_log_alert::alert_type:
	{
		LOG_DEBUG("dhtd::process_alerts", "received alert type dht_log_alert (%d)", lt::dht_log_alert::alert_type);
	}
	break;

	case lt::dht_pkt_alert::alert_type:
	{
		LOG_DEBUG("dhtd::process_alerts", "received alert type dht_pkt_alert (%d)", lt::dht_pkt_alert::alert_type);
	}
	break;

	case lt::dht_get_peers_reply_alert::alert_type:
	{
		LOG_DEBUG("dhtd::process_alerts", "received alert type dht_get_peers_reply_alert (%d)", lt::dht_get_peers_reply_alert::alert_type);
	}
	break;

	case lt::dht_direct_response_alert::alert_type:
	{
		LOG_DEBUG("dhtd::process_alerts", "received alert type dht_direct_response_alert (%d)", lt::dht_direct_response_alert::alert_type);
	}
	break;

	case lt::dht_live_nodes_alert::alert_type:
	{
		LOG_DEBUG("dhtd::process_alerts", "received alert type dht_live_nodes_alert (%d)", lt::dht_live_nodes_alert::alert_type);
	}
	break;

	default: {
		LOG_DEBUG("dhtd::process_alerts", "received unknown alert type (%d)", a->type());
	}
	}
	return res;
}



// Funnction to get the ID by URL(returns - 1 if not found)
int get_dht_reply_id_by_url(const std::string & url) {
	for (const auto& entry : dht_replies) {
		if (entry.second.url == url) {
			return entry.first;  // Return existing ID
		}
	}
	return -1;  // Not found
}

// Function to get the URL by ID (returns empty string if not found)
std::string get_dht_reply_url_by_id(int id) {
	auto it = dht_replies.find(id);
	if (it != dht_replies.end()) {
		return it->second.url;
	}
	return "";  // Not found
}

void process_alerts(lt::session& s) {
	auto last_log_time = std::chrono::steady_clock::now(); // Track last log time

	while (true) {
		std::vector<lt::alert*> alerts;
		s.pop_alerts(&alerts);

		bool has_alerts = false; // Track if alerts were processed

		for (auto const* a : alerts) {
			
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			has_alerts = true;

			bool pre_processed = pre_process_alert(a); // Debug function
			if (!pre_processed) {
				INFOLOG("alert failed preprocess.");
				continue;
			}
			if (auto dht_a = lt::alert_cast<lt::dht_announce_alert>(a)) {
				INFOLOG("[ DHT ANNOUNCE ]\t\t[%s] %s", dht_a->info_hash.to_string().c_str(), dht_a->ip.to_string().c_str());

			}
			else if (auto dht_r = lt::alert_cast<lt::dht_reply_alert>(a)) {
				INFOLOG("[ DHT REPLY ]\t\t[%s] -> %d", dht_r->url.c_str(), dht_r->num_peers);
			}

			// Always check if it's time to log
			auto now = std::chrono::steady_clock::now();
			if (std::chrono::duration_cast<std::chrono::seconds>(now - last_log_time).count() >= 5) {
				INFOLOG("[%s] waiting for network notifications...", get_current_datetime().c_str());
				last_log_time = now; // Reset the timer
			}

			// If no alerts were processed, wait for a short time to avoid busy looping
			if (!has_alerts) {
				std::this_thread::sleep_for(std::chrono::milliseconds(250)); // Lower CPU usage but faster loop
			}
		}
	}
}


