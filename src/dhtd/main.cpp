
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

#ifndef DEJA_DISABLED
#include "DejaLib.h"
#endif


using namespace std;
using namespace std::chrono_literals;

#pragma message( "Compiling " __FILE__ )
#pragma message( "Last modified on " __TIMESTAMP__ )



// ===============================================
// DHTReplyData : Struct to store DHT Reply Data
// ===============================================
struct DHTReplyData {
	std::string url;
	int num_peers;
};

// ===============================================
// Global storage for DHT replies
// ===============================================
std::map<int, DHTReplyData> dht_replies;
int dht_reply_id = 0;  // Auto-incrementing ID

// ===============================================
// DHTReplyData Accessors 
// ===============================================
int get_dht_reply_id_by_url(const std::string& url);
std::string get_dht_reply_url_by_id(int id);



bool pre_process_alert(const lt::alert* a);
void process_alerts(lt::session& s);
int dump_config_values();
bool init_settings_from_cfgfile(lt::settings_pack& settings);
bool init_settings_hardcoded(lt::settings_pack& settings);

uint8_t rr = 3;

int main(int argc, TCHAR** argv, TCHAR envp)
{

#ifdef UNICODE
	char** argn = (char**)Convert::allocate_argn(argc, argv);
#else
	char** argn = argv;
#endif // UNICODE

	CmdLineUtil::get()->initialize(argc, argn);

	CmdlineParser* inputParser = CmdLineUtil::get()->parser();

	SCmdlineOptValues helpOptionT(       { "-h", "--help" }     , "Display help information" , cmdOpT::Help);
	SCmdlineOptValues verboseOptionT(    { "-v", "--verbose" }  , "verbose output"           , cmdOpT::Verbose);
	SCmdlineOptValues noBannerOptionT(   { "-n", "--nobanner" } , "no banner"                , cmdOpT::NoBanner);
	SCmdlineOptValues quietOptionT(      { "-q", "--quiet" }    , "quiet"                    , cmdOpT::Quiet);
	SCmdlineOptValues cfgfilePathOptionT({ "-c", "--config" }   , "config file"              , cmdOpT::Config);



	CmdlineOption cmdlineOptionHelp(helpOptionT);
	CmdlineOption cmdlineOptionVerbose(verboseOptionT);
	CmdlineOption cmdlineOptionNoBanner(noBannerOptionT);
	CmdlineOption cmdlineOptionQuiet(quietOptionT);
	CmdlineOption cmdlineOptionConfigFile(cfgfilePathOptionT);

	inputParser->addOption(helpOptionT);
	inputParser->addOption(verboseOptionT);
	inputParser->addOption(noBannerOptionT);
	inputParser->addOption(quietOptionT);
	inputParser->addOption(cfgfilePathOptionT);

	bool optHelp = inputParser->isSet(cmdlineOptionHelp);
	bool optVerbose = inputParser->isSet(cmdlineOptionVerbose);
	bool optNoBanner = inputParser->isSet(cmdlineOptionNoBanner);
	bool optQuiet = inputParser->isSet(cmdlineOptionQuiet);
	bool optConfig = inputParser->isSet(cmdlineOptionConfigFile);

	//if (optQuiet) {
	//	g_is_verbose = false;
	//}
	
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

			bool is_valid = Config::get().validate(tmp_config_file);
			if (is_valid) {
				LOG_TRACE("main::ValidateConfigChecksum", "tmp_config_file %s VALID", tmp_config_file.c_str());
			}else {
				LOG_WARNING("main::ValidateConfigChecksum", "tmp_config_file %s NOT VALID", tmp_config_file.c_str());
#ifdef _RELEASE 
				throw("Error: Invalid config file");
				return -3;
#endif 
			}

			Config::get().initialize(tmp_config_file);
		}
		else {
			LOG_TRACE("main::arguments::optTracker", "missing url");
			usage_error("missing config Path");
			return -1;
		}
	}
	else {
		Config::get().initialize();
	}

	dump_config_values();

	lt::settings_pack settings;
	init_settings_from_cfgfile(settings);
	
	std::string test_listen_ifaces =  settings.get_str(lt::settings_pack::listen_interfaces);
	NOTICELOG("CHECK FOR SETTINGS VALIDITY: %s", test_listen_ifaces.c_str());
	lt::session s(settings);

	// Start processing DHT alerts
	process_alerts(s);

	if (CONFIG.debug_mode_enabled()) {

		LOG_DEBUG("main", "Sleeping for %d", CONFIG.dbg_exit_delay());
		Sleep(CONFIG.dbg_exit_delay());
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
		}
		// Always check if it's time to log
		auto now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::seconds>(now - last_log_time).count() >= 5) {
			INFOLOG("[%s] waiting for network notifications...", get_current_datetime().c_str());
			last_log_time = now; // Reset the timer
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Lower CPU usage but faster loop
	}
}


//==============================================================================
//
// function name: init_settings_from_cfgfile
// description:   this is an important function because for this program to
// actually execute what is expected of it, the network connections need to 
// be operational ando do all the configuration related to it.
// 
// The libtorrent library initialization uses this 'settings_pack' data structure
// and it is created here. 
// 
// NOTE: This **config** version of this function.
//       All values are dynamically assigned from the configuration file.
//============================================================================
// Copyright (C)  Guillaume Plante <codegp@icloud.com>
//==============================================================================

bool init_settings_from_cfgfile(lt::settings_pack &settings) {
	bool res = true;
	try {
		settings.set_bool(lt::settings_pack::enable_dht, true);
		settings.set_bool(lt::settings_pack::dht_restrict_routing_ips, false);
		settings.set_bool(lt::settings_pack::dht_prefer_verified_node_ids, false);

		settings.set_bool(lt::settings_pack::dht_aggressive_lookups, true);
		settings.set_bool(lt::settings_pack::dht_privacy_lookups, true);
		settings.set_bool(lt::settings_pack::dht_enforce_node_id, false);
		settings.set_bool(lt::settings_pack::dht_ignore_dark_internet, true);
		settings.set_bool(lt::settings_pack::validate_https_trackers, false);
		settings.set_bool(lt::settings_pack::use_dht_as_fallback, false);
		
		settings.set_bool(lt::settings_pack::enable_incoming_utp, CONFIG.net_incoming_utp());
		settings.set_bool(lt::settings_pack::enable_outgoing_utp, CONFIG.net_outgoing_utp());
		settings.set_bool(lt::settings_pack::enable_incoming_tcp, CONFIG.net_incoming_tcp());
		settings.set_bool(lt::settings_pack::enable_outgoing_tcp, CONFIG.net_outgoing_tcp());
		settings.set_str( lt::settings_pack::listen_interfaces  , CONFIG.net_listen_ifaces());
		settings.set_str( lt::settings_pack::outgoing_interfaces, CONFIG.net_outgoing_ifaces());
		settings.set_str( lt::settings_pack::dht_bootstrap_nodes, CONFIG.net_bootstrap_nodes());
		//settings.set_int(lt::settings_pack::dht_announce_interval,5);
		INFOLOG("=======================================");
		NOTICELOG("libtorrent settings");
		INFOLOG("enable_incoming_utp: %s", CONFIG.net_incoming_utp() ? "true" : "false");
		INFOLOG("enable_outgoing_utp: %s", CONFIG.net_outgoing_utp() ? "true" : "false");
		INFOLOG("Listen interface   \"%s\"", CONFIG.net_listen_ifaces().c_str());
		INFOLOG("Listen interface   \"%s\"", CONFIG.net_outgoing_ifaces().c_str());
		INFOLOG("bootstrap_nodes:   \"%s\"", CONFIG.net_bootstrap_nodes().c_str());
		INFOLOG("=======================================");
	}
	catch (const std::exception& e) {
		LOG_TRACE("main", "Exception occurred: %s", e.what());
		res = false;
	}

	return res;
}



//==============================================================================
//
// function name: init_settings_hardcoded
// description:   this is an important function because for this program to
// actually execute what is expected of it, the network connections need to 
// be operational ando do all the configuration related to it.
// 
// The libtorrent library initialization uses this 'settings_pack' data structure
// and it is created here. 
// 
// NOTE: This **HARDCODED** version of this function was written for 
// **debugging purposes only** The way it's supposed to work is that all
// values are to be dynamically assigned from the configuration file.
//============================================================================
// Copyright (C)  Guillaume Plante <codegp@icloud.com>
//==============================================================================

bool init_settings_hardcoded(lt::settings_pack& settings) {

	bool res = true;
	try {
		settings.set_bool(lt::settings_pack::enable_dht, true);
		settings.set_bool(lt::settings_pack::enable_incoming_utp, true);
		settings.set_bool(lt::settings_pack::enable_outgoing_tcp, true);
		settings.set_bool(lt::settings_pack::enable_incoming_tcp, true);
		settings.set_bool(lt::settings_pack::enable_outgoing_utp, true);
		//settings.set_int(lt::settings_pack::dht_announce_interval,5);
		//settings.set_str(lt::settings_pack::outgoing_interfaces, "10.0.0.138:6881");
		settings.set_str(lt::settings_pack::listen_interfaces, "10.0.0.138:6881");
		//settings.set_str(lt::settings_pack::dht_bootstrap_nodes, "router.bittorrent.com:6881,router.utorrent.com:6881,dht.transmissionbt.com:6881");
		settings.set_str(lt::settings_pack::dht_bootstrap_nodes, "router.bittorrent.com:6881");
	}
	catch (const std::exception& e) {
		LOG_TRACE("main", "Exception occurred: %s", e.what());
		res = false;
	}

	return res;
}


int dump_config_values() {

	try {
		Config& config = Config::get();
		CONFIG_LOG("ipv6_enabled:         %s", CONFIG.net_ipv6_enabled() ? "true" : "false");
		CONFIG_LOG("enable_incoming_utp:  %s", CONFIG.net_incoming_utp() ? "true" : "false");
		CONFIG_LOG("enable_outgoing_utp:  %s", CONFIG.net_outgoing_utp() ? "true" : "false");
		CONFIG_LOG("listen interface     \"%s\"", CONFIG.net_listen_ifaces().c_str());
		CONFIG_LOG("outgoing interface   \"%s\"", CONFIG.net_outgoing_ifaces().c_str());
		CONFIG_LOG("bootstrap_nodes:     \"%s\"", CONFIG.net_bootstrap_nodes().c_str());

		CONFIG_LOG("Console Logging: %s", config.log_to_console() ? "Enabled" : "Disabled");
		CONFIG_LOG("Log File: %s", config.logfile_path().c_str());

		CONFIG_LOG("Debug Mode: %s", config.debug_mode_enabled() ? "Enabled" : "Disabled");
		CONFIG_LOG("Exit Pause: %d seconds", config.dbg_exit_delay());
	}
	catch (const std::exception& e) {
		LOG_TRACE("main", "Exception occurred: %s", e.what());
	}

	return 0;
}