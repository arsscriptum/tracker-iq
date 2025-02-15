
//==============================================================================
//
//     config.h
//
//============================================================================
// Copyright (C)  Guillaume Plante <codegp@icloud.com>
//==============================================================================

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <string>
#include <memory>
#include <mutex>
#include "utils.h"

class Config {
public:

    Config();
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    static Config& getInstance();

    std::string getFilePath() const;
    static std::string getDefaultConfigPath();

    bool Initialize(const std::string configPath = "");
    bool ValidateConfigChecksum(const std::string& configPath);  // New validation function
    void Defaults();
    bool SetConfigChecksum(const std::string& configPath);

    bool net_ipv6_enabled() const;
    bool net_incoming_utp() const;
    bool net_outgoing_utp() const;

    std::string net_user_agent() const;
    std::string net_client_name() const;
    std::string net_peer_fingerprint() const;

    std::string net_listen_ifaces() const;
    std::string net_outgoing_ifaces() const;
    std::string net_bootstrap_nodes() const;

    // Logging
    bool isConsoleEnabled() const;
    bool isLogFileEnabled() const;
    std::string getLogFile() const;

    // Debug
    bool isDebugEnabled() const;
    unsigned int getDebugPause() const;
    unsigned int getExitPause() const;


private:


    bool ParseConfig(const std::string& configPath);
    
    bool _initialized;
    explicit Config(const std::string& filePath);

    static std::unique_ptr<Config> instance;
    static std::mutex mutex;

    std::string _filePath;

    // network 

    bool _enable_ipv6;
    bool _enable_outgoing_utp;
    bool _enable_incoming_utp;

    // this is the client identification to the tracker. The recommended
    // format of this string is: "client-name/client-version
    // libtorrent/libtorrent-version". This name will not only be used when
    // making HTTP requests, but also when sending extended headers to
    // peers that support that extension. It may not contain \r or \n
    std::string _user_agent;
    // this is the client name and version identifier sent to peers in the
    // handshake message. If this is an empty string, the user_agent is
    // used instead. This string must be a UTF-8 encoded unicode string.
    std::string _client_name;


    // this is the fingerprint for the client. It will be used as the
    // prefix to the peer_id. If this is 20 bytes (or longer) it will be
    // truncated to 20 bytes and used as the entire peer-id
    //
    // There is a utility function, generate_fingerprint() that can be used
    // to generate a standard client peer ID fingerprint prefix.
    std::string _peer_fingerprint;
    // This is a comma-separated list of IP port-pairs. They will be added
    // to the DHT node (if it's enabled) as back-up nodes in case we don't
    // know of any.
    //
    // Changing these after the DHT has been started may not have any
    // effect until the DHT is restarted.
    std::string _bootstrap_nodes;

    // This controls which IP address outgoing TCP peer connections are bound
    // to, in addition to controlling whether such connections are also
    // bound to a specific network interface/adapter (*bind-to-device*).
    //
    // This string is a comma-separated list of IP addresses and
    // interface names. An empty string will not bind TCP sockets to a
    // device, and let the network stack assign the local address.
    std::string _outgoing_ifaces;

    // a comma-separated list of (IP or device name, port) pairs. These are
    // the listen ports that will be opened for accepting incoming uTP and
    // TCP peer connections. These are also used for *outgoing* uTP and UDP
    // tracker connections and DHT nodes.
    std::string _listen_ifaces;

    // [debug]   
    bool _debug_enabled;
    unsigned int _debug_pause;
    unsigned int _exit_pause;

    // [log]
    bool _enable_console;
    bool _enable_logfile;

    std::string _logfile;
    // Function to replace $ENV:VAR with its value
    static std::string resolveEnvVariables(const std::string& value);
    // Function to compute checksum
    static std::string ComputeFileChecksum(const std::string& filePath);
#ifdef __TIQ_IMPL__
    TrackerType _tracker_type;
#endif // __TIQ_IMPL__
};

#define CONFIG Config::getInstance()


#endif // __CONFIG_H__
