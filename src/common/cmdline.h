//==============================================================================
//
//     cmdline.h
//
//============================================================================
// Copyright (C)  Guillaume Plante <codegp@icloud.com>
//==============================================================================

#ifndef __CMDLINE_H__
#define __CMDLINE_H__

#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include <algorithm>
#include "log.h"

#include "cmdline_opt_values.h"

// New modern C++ structure with a constructor for easier initialization.
struct SCmdlineOptValues {
    std::vector<std::string> options;
    std::string description;
    std::string uid;

    // Constructor to initialize all members.
    SCmdlineOptValues(const std::vector<std::string>& opts,
        const std::string& desc,
        const std::string& id)
        : options(opts), description(desc), uid(id) {}
};

class CmdlineOption
{
public:
    // Constructor taking options and description. _uid can be set later.
    CmdlineOption(const std::vector<std::string>& opt, const std::string& desc)
        : _options(opt), _description(desc) {}

    // New constructor that accepts an SCmdlineOptValues instance.
    CmdlineOption(const SCmdlineOptValues& optValues)
        : _options(optValues.options),
        _description(optValues.description),
        _typeId(optValues.type)
    {}

    bool isValid(std::string option) {
        for (auto&& str : _options) {
            if (str == option) { return true; }
        }
        return false;
    }
    bool operator == (const CmdlineOption& other) const
    {
        return _options == other._options;
    }
    void dump_options(std::ostringstream& dbg_output) {
        for (auto&& opt : _options) {
            dbg_output << _description << " : " << opt << std::endl;
            LOG_TRACE("CmdlineOption::dump_options", "%s: option %s", _description.c_str(), opt.c_str());
        }
    }

    std::vector<std::string> _options;
    std::string _description;
    cmdOpT _typeId;
};


// Class to manage program arguments
class CmdlineParser
{
public:
   
    CmdlineParser() {};
    void reset(int& argc, char** argv)
    {
        for (int i = 1; i < argc; ++i) {
            this->tokens.push_back(std::string(argv[i]));
        }
    }

    const std::string& getCmdOption(const std::string& option) const {
        LOG_TRACE("cmdline::getCmdOption", "option %s", option.c_str());
        std::vector<std::string>::const_iterator itr;
        itr = std::find(this->tokens.begin(), this->tokens.end(), option);
        if (itr != this->tokens.end() && ++itr != this->tokens.end())
        {
            std::string tmpstr = *itr;
            LOG_TRACE("cmdline::getCmdOption", "FOUND option %s", tmpstr.c_str());
            return *itr;
        }
        static const std::string empty_string("");
        return empty_string;
    }

    bool get_option_argument(const CmdlineOption& cmdopt, std::string& value) {
        LOG_TRACE("cmdline::get_option_argument", "%s", cmdopt._description.c_str());
        for (auto&& opt : cmdopt._options) {
            LOG_TRACE("cmdline::get_option_argument", "check %s", opt.c_str());
            if (cmdOptionExists(opt) == true) {
                LOG_TRACE("cmdline::get_option_argument", "cmdOptionExists %s == true", opt.c_str());
                std::string retval = getCmdOption(opt);
                LOG_TRACE("cmdline::get_option_argument", "getCmdOption %s == %s", opt.c_str(), retval.c_str());
                if (retval.length() > 0) {
                    value = retval;
                    return true;
                }
            }
        }
        LOG_TRACE("cmdline::get_option_argument", "not found");
        return false;
    }

    bool cmdOptionExists(const std::string& option) const
    {
        return std::find(this->tokens.begin(), this->tokens.end(), option) != this->tokens.end();
    }

    // Updated addOption that takes the new structure type.
    void addOption(const SCmdlineOptValues& optVal)
    {
        CmdlineOption option(optVal.options, optVal.description);
        option._uid = optVal.uid;
        options.push_back(option);
    }

    bool isSet(CmdlineOption& cmdlineOption)
    {
        std::ostringstream dbg_output;
        cmdlineOption.dump_options(dbg_output);
        LOG_TRACE("CmdlineOption::isSet", "%s %s", cmdlineOption._description.c_str(), dbg_output.str().c_str());
        for (auto&& opt : cmdlineOption._options) {
            if (cmdOptionExists(opt) == true) {
                return true;
            }
        }
        return false;
    }
    bool invalidToken() {
        for (auto&& token : tokens) {
            if (cmdlineOptionValid(token) == false) {
                return true;
            }
        }
        return false;
    }

    void dump_tokens(std::ostringstream& dbg_output) {
        for (auto&& token : tokens) {
            dbg_output << token << std::endl;
        }
    }
private:
    bool cmdlineOptionValid(std::string opt)
    {
        for (auto&& elem : options) {
            if (elem.isValid(opt)) {
                return true;
            }
        }
        return false;
    }
    std::vector <std::string> tokens;
    std::vector<CmdlineOption> options;

    std::map<int, DHTReplyData> dht_replies;
};

class CmdLineUtil
{
public:
    static CmdLineUtil* get();

    void initialize(int argc, char** argv) {
        inputParser.reset(argc, argv);
    };
    CmdlineParser* parser() { return &inputParser; };

private:
    CmdLineUtil() {};

    CmdlineParser inputParser;
    static CmdLineUtil* instance;
};

#endif //__CMDLINE_H__

