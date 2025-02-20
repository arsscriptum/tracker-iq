
//==============================================================================
//
//     config.cpp
//
//============================================================================
// Copyright (C)  Guillaume Plante <codegp@icloud.com>
//==============================================================================

#include "stdafx.h"
#include "ini.h"
#include "inireader.h"
#include "log.h"
#include "runner.h"
#include "config.h"
#include <iostream>
#include <cstdlib>  // For getenv()
#include <regex>
#include <filesystem>
#include <vector>
#include <random>
#include <libtorrent/sha1_hash.hpp> // make sure to include the appropriate libtorrent header
#include <libtorrent/session.hpp>
#include <libtorrent/address.hpp>
#include <libtorrent/socket.hpp>  // for udp::endpoint


#include <boost/asio/ip/address.hpp>

Runner::Runner(lt::settings_pack* settings)
	: ses(*settings)  // Directly construct session object using settings
{
	LOG_DEBUG("Runner::Runner","Runner initialized");
}


bool Runner::CheckDhtRunning() {
	bool is_running = ses.is_dht_running();
	LOG_DEBUG("Runner::CheckDhtRunning", "Called is_dht_running: %s", is_running?"yes":"no");

	addCustomDhtNode("82.221.103.244", 6881);
	addCustomDhtNode("212.129.33.59", 6881);
	addCustomDhtNode("67.215.246.10", 6881);
	return is_running;
}
void Runner::GetPeers()
{
	LOG_DEBUG("Runner::GetPeers", "Runner GetPeers");

	// List of info_hashes available
	std::vector<lt::sha1_hash> infoHashes = {
		lt::sha1_hash("8733c9e41939da2dbd23937d058f030dd9bb1c73"),
		lt::sha1_hash("8733c9e41939da2dbd23937d9c7c86070476ba58"),
		lt::sha1_hash("8733c9e41939da2dbd23937dc207cfc8aabe83d4"),
		lt::sha1_hash("8733c9e41939da2dbd23937d69883d87252e3d23"),
		lt::sha1_hash("17f17450518cb295d6d91266416f0b4c0f8996b7"),
		lt::sha1_hash("17f17450518cb295d6d912662d4db1cec15647cf"),
		lt::sha1_hash("17f17450518cb295d6d91266ca2d204c79f37914"),
		lt::sha1_hash("17f17450518cb295d6d912668d91646a8e7d331f")
	};

	// Set up a random device and generator
	static std::random_device rd;         // Seed for randomness
	static std::mt19937 gen(rd());          // Mersenne Twister RNG
	std::uniform_int_distribution<> dis(0, infoHashes.size() - 1);

	// Select a random index
	int index = dis(gen);
	lt::sha1_hash selectedHash = infoHashes[index];

	// Issue the get_peers query with the randomly selected hash.
	ses.dht_get_peers(selectedHash);

	LOG_DEBUG("Runner::GetPeers", "Called dht_get_peers with hash: %s", selectedHash.to_string().c_str());
}

void Runner::addCustomDhtNode( const std::string& ipStr, unsigned short port)
{
	try {
		LOG_DEBUG("Runner::addCustomDhtNode", "addCustomDhtNode: %s", ipStr.c_str());

		// Convert string to libtorrent address (IPv4 or IPv6)
		lt::address ip = boost::asio::ip::make_address(ipStr.c_str());
		// Create a pair: first element is the IP as a string, second element is the port.
		std::pair<std::string, int> node(ip.to_string(), port);

		// Add the DHT node.
		ses.add_dht_node(node);
		std::cout << "Added DHT node: " << ipStr << ":" << port << std::endl;
	}
	catch (std::exception& e) {
		std::cerr << "Error adding DHT node: " << e.what() << std::endl;
	}
}

bool Runner::pre_process_alert(const lt::alert* a) {
	bool res = true;
	switch (a->type())
	{
	case lt::dht_sample_infohashes_alert::alert_type:
	{
		LOG_DEBUG("Runner::process_alerts", "received alert type dht_sample_infohashes_alert (%d)", lt::dht_sample_infohashes_alert::alert_type);
	}
	break;

	case lt::dht_outgoing_get_peers_alert::alert_type:
	{
		LOG_DEBUG("Runner::process_alerts", "received alert type dht_outgoing_get_peers_alert (%d)", lt::dht_outgoing_get_peers_alert::alert_type);
	}
	break;

	case lt::dht_bootstrap_alert::alert_type:
	{
		LOG_DEBUG("Runner::process_alerts", "received alert type dht_bootstrap_alert (%d)", lt::dht_bootstrap_alert::alert_type);
	}
	break;

	case lt::dht_reply_alert::alert_type:
	{
		LOG_DEBUG("Runner::process_alerts", "received alert type dht_reply_alert (%d)", lt::dht_reply_alert::alert_type);

		if (auto dhtReply = lt::alert_cast<lt::dht_reply_alert>(a)) {
		}
	}
	break;

	case lt::dht_announce_alert::alert_type:
	{
		LOG_DEBUG("Runner::process_alerts", "received alert type dht_announce_alert (%d)", lt::dht_announce_alert::alert_type);
	}
	break;

	case lt::dht_get_peers_alert::alert_type:
	{
		LOG_DEBUG("Runner::process_alerts", "received alert type dht_get_peers_alert (%d)", lt::dht_get_peers_alert::alert_type);
	}
	break;

	case lt::dht_error_alert::alert_type:
	{
		LOG_DEBUG("Runner::process_alerts", "received alert type dht_error_alert (%d)", lt::dht_error_alert::alert_type);
		auto dht_e = lt::alert_cast<lt::dht_error_alert>(a);
		ERROR_DESC("dhtd::process_alerts", "operation id %d. %s %s", (unsigned int)dht_e->operation, dht_e->error.message().c_str(), dht_e->error.to_string().c_str());
		res = false;
	}
	break;
	case lt::portmap_error_alert::alert_type:
	{
		LOG_DEBUG("Runner::process_alerts", "received alert type portmap_error_alert (%d)", lt::portmap_error_alert::alert_type);
		auto dht_e = lt::alert_cast<lt::portmap_error_alert>(a);
		ERROR_DESC("dhtd::process_alerts", "what ? \"%s\" . %s %s. transport %d. mapping %d.", dht_e->what(), dht_e->error.message().c_str(), dht_e->error.to_string().c_str(), dht_e->local_address.to_string().c_str(), (unsigned int)dht_e->map_transport, (int)dht_e->mapping);
		res = false;
	}
	break;

	case lt::udp_error_alert::alert_type:
	{
		LOG_DEBUG("Runner::process_alerts", "received alert type udp_error_alert (%d)", lt::udp_error_alert::alert_type);
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
		LOG_DEBUG("Runner::process_alerts", "received alert type dht_immutable_item_alert (%d)", lt::dht_immutable_item_alert::alert_type);
	}
	break;

	case lt::dht_mutable_item_alert::alert_type:
	{
		LOG_DEBUG("Runner::process_alerts", "received alert type dht_mutable_item_alert (%d)", lt::dht_mutable_item_alert::alert_type);
	}
	break;

	case lt::dht_put_alert::alert_type:
	{
		LOG_DEBUG("Runner::process_alerts", "received alert type dht_put_alert (%d)", lt::dht_put_alert::alert_type);
	}
	break;

	case lt::dht_stats_alert::alert_type:
	{
		LOG_DEBUG("Runner::process_alerts", "received alert type dht_stats_alert (%d)", lt::dht_stats_alert::alert_type);
	}
	break;

	case lt::dht_log_alert::alert_type:
	{
		LOG_DEBUG("Runner::process_alerts", "received alert type dht_log_alert (%d)", lt::dht_log_alert::alert_type);
	}
	break;

	case lt::dht_pkt_alert::alert_type:
	{
		LOG_DEBUG("Runner::process_alerts", "received alert type dht_pkt_alert (%d)", lt::dht_pkt_alert::alert_type);
	}
	break;

	case lt::dht_get_peers_reply_alert::alert_type:
	{
		LOG_DEBUG("Runner::process_alerts", "received alert type dht_get_peers_reply_alert (%d)", lt::dht_get_peers_reply_alert::alert_type);
	}
	break;

	case lt::dht_direct_response_alert::alert_type:
	{
		LOG_DEBUG("Runner::process_alerts", "received alert type dht_direct_response_alert (%d)", lt::dht_direct_response_alert::alert_type);
	}
	break;

	case lt::dht_live_nodes_alert::alert_type:
	{
		LOG_DEBUG("Runner::process_alerts", "received alert type dht_live_nodes_alert (%d)", lt::dht_live_nodes_alert::alert_type);
	}
	break;

	default: {
		LOG_DEBUG("Runner::process_alerts", "received unknown alert type (%d)", a->type());
	}
	}
	return res;
}


void Runner::process_alerts() {
	auto last_log_time = std::chrono::steady_clock::now(); // Track last log time

	while (true) {
		std::vector<lt::alert*> alerts;
		ses.pop_alerts(&alerts);

		bool has_alerts = false; // Track if alerts were processed

		for (auto const* a : alerts) {

			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			has_alerts = true;

			bool pre_processed = pre_process_alert(a); // Debug function
			if (!pre_processed) {
				INFOLOG("alert failed preprocess.");
				//continue;
			}
			if (auto dht_a = lt::alert_cast<lt::dht_announce_alert>(a)) {
				INFOLOG("[ DHT ANNOUNCE ]\t\t[%s] %s", dht_a->info_hash.to_string().c_str(), dht_a->ip.to_string().c_str());

			}
			else if (auto dht_r = lt::alert_cast<lt::dht_reply_alert>(a)) {
				INFOLOG("[ DHT REPLY ]\t\t[%s] -> %d", dht_r->url.c_str(), dht_r->num_peers);
			}
			else {
				INFOLOG("[ DHT REPLY ] received unknown alert type (%d)", a->type());
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
unsigned long Runner::Process(void* parameter) 
{
	process_alerts();
	return 0;
}


void Runner::process_dht_reply_alert(const lt::dht_reply_alert* dhtReply)
{
	if (dhtReply == nullptr) {
		return;
	}


	// Log basic information.
	INFOLOG("[DHT REPLY] tracker_url: %s, torrent_name: %s, num_peers %d ", dhtReply->tracker_url(), dhtReply->torrent_name(), dhtReply->num_peers);

	// Assuming dhtReply->nodes is a std::string containing the compact node info.
	/*std::string nodesPayload = dhtReply->n;  // Adjust this line if your API differs.
	if (!nodesPayload.empty()) {
		auto peers = parseDHTNodes(nodesPayload);
		std::cout << "[DHT REPLY] Received " << peers.size() << " peers:" << std::endl;
		for (const auto& peer : peers) {
			std::cout << "   Peer IP: " << peer.ip << ", Port: " << peer.port << std::endl;
			// Optionally, log the node ID as well:
			// std::cout << "   Node ID: " << peer.nodeId << std::endl;
		}
	}
	else {
		std::cout << "[DHT REPLY] No nodes payload available." << std::endl;
	}*/

}


std::vector<Runner::DHTNode> Runner::parseDHTNodes(const std::string& nodes) {
	std::vector<DHTNode> result;
	const size_t nodeSize = 26;
	for (size_t i = 0; i + nodeSize <= nodes.size(); i += nodeSize) {
		DHTNode node;
		// Extract node id (20 bytes)
		node.nodeId = nodes.substr(i, 20);

		// Extract IP (4 bytes)
		uint32_t ipRaw;
		memcpy(&ipRaw, nodes.data() + i + 20, 4);
		// The IP is in network byte order; use inet_ntoa to convert to string.
		struct in_addr addr;
		addr.s_addr = ipRaw;
		node.ip = inet_ntoa(addr);

		// Extract port (2 bytes) and convert from network to host order.
		uint16_t portRaw;
		memcpy(&portRaw, nodes.data() + i + 24, 2);
		node.port = ntohs(portRaw);

		result.push_back(node);
	}
	return result;
}


// Funnction to get the ID by URL(returns - 1 if not found)
int Runner::get_dht_reply_id_by_url(const std::string& url) {
	for (const auto& entry : dht_replies) {
		if (entry.second.url == url) {
			return entry.first;  // Return existing ID
		}
	}
	return -1;  // Not found
}

// Function to get the URL by ID (returns empty string if not found)
std::string Runner::get_dht_reply_url_by_id(int id) {
	auto it = dht_replies.find(id);
	if (it != dht_replies.end()) {
		return it->second.url;
	}
	return "";  // Not found
}
