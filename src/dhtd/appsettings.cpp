
//==============================================================================
//
//     appsettings.cpp
//
//============================================================================
// Copyright (C)  Guillaume Plante <codegp@icloud.com>
//==============================================================================

#include "stdafx.h"
#include "log.h"
#include "appsettings.h"
#include <iostream>
#include <cstdlib>  // For getenv()
#include <regex>
#include <filesystem>
#include "cmdline.h"

// Initialize static members
std::unique_ptr<AppSettings> AppSettings::instance = nullptr;
std::mutex AppSettings::mutex;


AppSettings::AppSettings() {
    defaults();
}

AppSettings& AppSettings::get() {
    std::lock_guard<std::mutex> lock(mutex);

    if (!instance) {
        instance = std::unique_ptr<AppSettings>(new AppSettings());
    }

    return *instance;
}


bool AppSettings::initialize(CmdlineParser* parser) {


    return _initialized;
}

void AppSettings::defaults() {

    cmdArgumentsParser = nullptr;
    _verbose = false;

}

bool AppSettings::verbose_mode() const {
    return _verbose;
}
