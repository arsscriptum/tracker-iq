
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
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
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
#include "runner.h"
#include "inireader.h"
#include "config.h"
#include "version.h"
#include "DejaLib.h"
#include <chrono>
#include <ctime>
#include <winsock2.h> // for ntohs, inet_ntoa. On Linux, use <arpa/inet.h>

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

volatile bool stop = 0; // Flag to indicate when to stop

BOOL WINAPI handle_ctrl_c(DWORD signal) {
	if (signal == CTRL_C_EVENT) {
		stop = 1; // Set flag when Ctrl-C is pressed
		return TRUE;
	}
	return FALSE;
}

int dump_config_values();
bool init_settings_from_cfgfile(lt::settings_pack& settings);
bool init_settings_hardcoded(lt::settings_pack& settings);


std::string toLocalDateTime(std::chrono::system_clock::time_point systemTime);

uint8_t rr = 3;

// At startup, capture both time points:
const std::chrono::steady_clock::time_point g_startupSteady = std::chrono::steady_clock::now();
std::chrono::system_clock::time_point g_startupSystem = std::chrono::system_clock::now();

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

	// Register the Ctrl-C handler
	SetConsoleCtrlHandler(handle_ctrl_c, TRUE);

	printf("Press Ctrl-C to exit...\n");
	Runner appRunner(&settings);
	appRunner.CreateThread();
	while (!stop) {
		if (_kbhit()) {  // Check if a key was pressed
			int key = _getch();  // Get the key

			if (key == '1') {
				appRunner.GetPeers();  // Convert char to int
			}
			else if (key == '2') {
				appRunner.CheckDhtRunning();
			}
		}

		Sleep(200); // Simulating work (1000ms = 1s)
	}

	printf("\nExiting gracefully...\n");


	if (CONFIG.debug_mode_enabled()) {

		LOG_DEBUG("main", "Sleeping for %d", CONFIG.dbg_exit_delay());
		Sleep(CONFIG.dbg_exit_delay());
	}
	
	return 0;
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
		INFOLOG("listen_interfaces     \"%s\"", CONFIG.net_listen_ifaces().c_str());
		INFOLOG("outgoing_interfaces   \"%s\"", CONFIG.net_outgoing_ifaces().c_str());
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



std::string toLocalDateTime(std::chrono::system_clock::time_point systemTime)
{
	std::time_t t = std::chrono::system_clock::to_time_t(systemTime);
	std::tm localTm;
#if defined(_WIN32) || defined(_WIN64)
	localtime_s(&localTm, &t);
#else
	localtime_r(&t, &localTm);
#endif
	std::ostringstream oss;
	oss << std::put_time(&localTm, "%Y-%m-%d %H:%M:%S");
	return oss.str();
}
