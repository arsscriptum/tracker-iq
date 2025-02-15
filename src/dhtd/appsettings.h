
//==============================================================================
//
//     appsettings.h
//
//============================================================================
// Copyright (C)  Guillaume Plante <codegp@icloud.com>
//==============================================================================

#ifndef __APPSETTINGS_H__
#define __APPSETTINGS_H__

#include <string>
#include <memory>
#include <mutex>
#include "utils.h"

class AppSettings {
public:

    AppSettings();
    AppSettings(const AppSettings&) = delete;
    AppSettings& operator=(const AppSettings&) = delete;

    static AppSettings& get();

    bool initialize(class CmdlineParser* parser);
    void defaults();
    
    
    bool verbose_mode() const;


private:

    bool _initialized;
    explicit AppSettings(const std::string& filePath);

    static std::unique_ptr<AppSettings> instance;
    static std::mutex mutex;

    bool _verbose;


    class CmdlineParser* cmdArgumentsParser;

};

#define SETTINGS AppSettings::get()


#endif // __APPSETTINGS_H__
