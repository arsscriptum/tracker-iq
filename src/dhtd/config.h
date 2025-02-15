
//==============================================================================
//
//     config.h
//
//============================================================================
//  Copyright (C) Guilaume Plante 2020 <cybercastor@icloud.com>
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


    // Getters for config values
    unsigned int getVersion() const;

    /* enable_ipv6 = false
enable_outgoing_utp = true 
enable_incoming_utp = true 
udp_port = 6881
bootstrap_nodes = router.bittorrent.com:6881,router.utorrent.com:6881,dht.transmissionbt.com:6881

*/
    bool net_ipv6_enabled() const;
    bool net_incoming_utp() const;
    bool net_outgoing_utp() const;
    int net_udp_port() const;
    std::string net_listen_address() const;
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

    // [general]
    unsigned int _version;

    // [trackers]
    unsigned int _timeout;
    unsigned int _num_want;
    std::string  _type;
    double _weight_peers;
    double _weight_ping;

    // [log]
    bool _enable_console;
    bool _enable_logfile;
    std::string _logfile;

    // network 
    bool _enable_ipv6;
    bool _enable_outgoing_utp;
    bool _enable_incoming_utp;
    int _udp_port;
    std::string _bootstrap_nodes;
    std::string _listen_address;

    // [debug]   
    bool _debug_enabled;
    unsigned int _debug_pause;
    unsigned int _exit_pause;

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
