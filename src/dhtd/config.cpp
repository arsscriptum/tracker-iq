
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
#include "utils.h"
#include "config.h"
#include <iostream>
#include <cstdlib>  // For getenv()
#include <regex>
#include <filesystem>

// Initialize static members
std::unique_ptr<Config> Config::instance = nullptr;
std::mutex Config::mutex;


Config::Config() {
    Defaults();
}

Config& Config::getInstance() {
    std::lock_guard<std::mutex> lock(mutex);

    if (!instance) {
        instance = std::unique_ptr<Config>(new Config());
    }

    return *instance;
}

std::string Config::getFilePath() const {
    return _filePath;
}

std::string Config::getDefaultConfigPath() {
    namespace fs = std::filesystem;

    fs::path currentPath = fs::current_path() / "dhtd.ini";
    if (fs::exists(currentPath)) {
        return currentPath.string();
    }

    fs::path configPath = fs::current_path() / "config" / "dhtd.ini";
    if (fs::exists(configPath)) {
        return configPath.string();
    }

    return "";
}

// Function to replace $ENV:VAR with its value
std::string Config::resolveEnvVariables(const std::string& value) {
    if (value.rfind("$ENV:", 0) == 0) {  // Check if it starts with "$ENV:"
        std::string envVarStr = value.substr(5); // Remove "$ENV:"
        size_t slashPos = envVarStr.find('\\');

        std::string envVar = envVarStr.substr(0, slashPos); // Extract env name
        const char* envValue = std::getenv(envVar.c_str());

        if (envValue) {
            std::string resolvedPath = envValue;
            if (slashPos != std::string::npos) { // If there's extra path after env
                resolvedPath += envVarStr.substr(slashPos);
            }
            return resolvedPath;
        }
    }
    return value; // Return original value if no match
}

bool Config::Initialize(const std::string path) {

    if (path.length() == 0) {
        std::string default_filepath = getDefaultConfigPath();
        if (default_filepath.length()) {
            ParseConfig(default_filepath);
            _filePath = path;
        }
    }
    else {
        ParseConfig(path);
        _filePath = path;
    }
    return _initialized;
}

// Function to set/update checksum in config file
bool Config::SetConfigChecksum(const std::string& configPath) {

    LOG_TRACE("Config::SetConfigChecksum", "%s", configPath.c_str());
    std::string newChecksum = ComputeFileChecksum(configPath);
    if (newChecksum.empty()) {
        return false;
    }

    std::ifstream file(configPath);
    if (!file) {
        return false;
    }

    std::ostringstream newContent;
    bool checksumSectionFound = false;
    std::string line;

    while (std::getline(file, line)) {
        if (line.find("[checksum]") != std::string::npos) {
            checksumSectionFound = true;
            newContent << "[checksum]\n";
            newContent << "hash = " << newChecksum << "\n";
            break;
        }
        newContent << line << "\n";
    }

    // Append checksum if it wasn't found
    if (!checksumSectionFound) {
        newContent << "\n[checksum]\n";
        newContent << "hash = " << newChecksum << "\n";
    }

    // Write updated content back to file
    std::ofstream outFile(configPath);
    if (!outFile) {
        return false;
    }

    outFile << newContent.str();
    return true;
}


// Function to compute SHA-256 checksum of a file
std::string Config::ComputeFileChecksum(const std::string& filePath) {
    LOG_TRACE("Config::ComputeFileChecksum", "%s", filePath.c_str());
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        return "";
    }

    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    char buffer[4096];
    while (file.read(buffer, sizeof(buffer))) {
        SHA256_Update(&sha256, buffer, file.gcount());
    }
    SHA256_Update(&sha256, buffer, file.gcount()); // Process last chunk

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &sha256);

    std::ostringstream oss;
    for (unsigned char c : hash) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    }

    return oss.str();
}

// Validate checksum before loading config
bool Config::ValidateConfigChecksum(const std::string& configPath) {
    LOG_TRACE("Config::ValidateConfigChecksum", "%s", configPath.c_str());

    INIReader reader(configPath);

    if (reader.ParseError() < 0) {
        app_error("Cannot load config file for checksum validation");
        return false;
    }

    // Retrieve expected checksum from file
    std::string expectedChecksum = reader.Get("checksum", "hash", "");

    // Compute actual checksum
    std::string actualChecksum = ComputeFileChecksum(configPath);

    if (actualChecksum.empty()) {
        app_error("Failed to compute config file checksum");
        return false;
    }
    LOG_TRACE("Config::ValidateConfigChecksum", "expectedChecksum %s actualChecksum %s", expectedChecksum.c_str(),actualChecksum.c_str());
    // Compare hashes
    if (expectedChecksum != actualChecksum) {
        app_error("Config file checksum mismatch!");
        return false;
    }
    
    return true;
}

void Config::Defaults() {

    _enable_ipv6 = false;
    _enable_outgoing_utp = true;
    _enable_incoming_utp = true;

    _user_agent = "";
    _client_name = "";

    _peer_fingerprint = "";
    _bootstrap_nodes = "";
    _outgoing_ifaces = "";
    _listen_ifaces = "";

    _enable_console = false;
    _enable_logfile = false;
    _logfile = "";

    _debug_enabled = false;
    _debug_pause = 0;
    _exit_pause = 0;
    _filePath = "";
    _initialized = false;
}

bool Config::ParseConfig(const std::string& configPath) {
    INIReader reader(configPath);

    if (reader.ParseError() < 0) {
        app_error("Cannot load config file");
        return false;
    }


    _user_agent = reader.Get("network", "user_agent", "");
    _client_name = reader.Get("network", "client_name", "");

    _peer_fingerprint = reader.Get("network", "peer_fingerprint", "");
    _bootstrap_nodes = reader.Get("network", "bootstrap_nodes", "");
    _outgoing_ifaces = reader.Get("network", "_outgoing_ifaces", "");
    _listen_ifaces = reader.Get("network", "listen_ifaces", "0");


    _enable_console = reader.GetBoolean("log", "console", false);
    _enable_logfile = reader.GetBoolean("log", "file", false);
    _logfile = resolveEnvVariables(reader.Get("log", "path", ""));

    _debug_enabled = reader.GetBoolean("debug", "enabled", false);
    _debug_pause = reader.GetInteger("debug", "debug_pause", 0);
    _exit_pause = reader.GetInteger("debug", "exit_pause", 0);
    _filePath = configPath;
    _initialized = true;
#ifdef __TIQ_IMPL__
    _tracker_type  = string_to_tracker_type(CONFIG.getTrackerTypeString());
#endif
    return _initialized;
}

// Getters for [log]
bool Config::isConsoleEnabled() const {
    return _enable_console;
}

bool Config::isLogFileEnabled() const {
    return _enable_logfile;
}

std::string Config::getLogFile() const {
    return _logfile;
}

// Getters for [debug]
bool Config::isDebugEnabled() const {
    return _debug_enabled;
}

unsigned int Config::getDebugPause() const {
    return _debug_pause;
}

unsigned int Config::getExitPause() const {
    return _exit_pause;
}

bool Config::net_ipv6_enabled() const  {
    return _enable_ipv6;
}
bool Config::net_incoming_utp() const  {
    return _enable_incoming_utp;
}
bool Config::net_outgoing_utp() const  {
    return _enable_outgoing_utp;
}
std::string Config::net_user_agent() const {
    return _user_agent;
}

std::string Config::net_client_name() const {
    return _client_name;
}
std::string Config::net_peer_fingerprint() const {
    return _peer_fingerprint;
}
std::string Config::net_bootstrap_nodes() const {
    return _bootstrap_nodes;
}
std::string Config::net_listen_ifaces() const {
    return _listen_ifaces;
}
std::string Config::net_outgoing_ifaces() const {
    return _outgoing_ifaces;
}