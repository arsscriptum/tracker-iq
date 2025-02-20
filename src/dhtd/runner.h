#ifndef __RUNNER_H__
#define __RUNNER_H__

//project dependencies

//win32 specific
#include <windows.h>
#include "cthread.h"
#include <vector>
#include <libtorrent/settings_pack.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/error_code.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/session_params.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_status.hpp>
#include <libtorrent/error_code.hpp>

//end project dependencies






class Runner: public CThread {

public:
	//constructors
	Runner(lt::settings_pack* settings);

	// Destructor
	~Runner() = default;  // No need to manually delete anything

	void GetPeers();
	bool CheckDhtRunning();

// ===============================================
// DHTNode : Structure to hold parsed node information
// ===============================================
	struct DHTNode {
		std::string nodeId;
		std::string ip;
		uint16_t port;
	};

	// ===============================================
	// DHTReplyData : Struct to store DHT Reply Data
	// ===============================================
	struct DHTReplyData {
		std::string url;
		int num_peers;
	};

protected:

	//overrideable
	unsigned long Process(void* parameter);
private:
	void process_dht_reply_alert(const lt::dht_reply_alert* dhtReply);
	std::vector<DHTNode> parseDHTNodes(const std::string& nodes);
	bool pre_process_alert(const lt::alert* a);
	void process_alerts();
	lt::session ses;

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
	void addCustomDhtNode(const std::string& ipStr, unsigned short port);

};


#endif // __RUNNER_H__