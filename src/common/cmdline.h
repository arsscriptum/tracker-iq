
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
#include "log.h"

class CmdlineOption
{
public:
	CmdlineOption(std::vector<std::string> opt, std::string desc){
		options = opt;
		description = desc;
	};
	bool isValid(std::string option){
		for (auto&& str : options){
			if (str == option) { return true; }
		}
		return false;
	}
	bool operator == (const CmdlineOption & other) const
	{
		return options == other.options;
	}
	void dump_options(std::ostringstream& dbg_output) {
		for (auto&& opt : options) {
			dbg_output << description.c_str() << " : " << opt << std::endl;

			LOG_TRACE("CmdlineOption::dump_options", "%s: option %s", description.c_str(), opt.c_str());
		}
	}
	
	std::vector<std::string> options;
	std::string description;
};

// Class to manage program arguments
class CmdlineParser
{
public:
	CmdlineParser() {};
	void reset(int &argc,  char **argv)
	{
		for (int i = 1; i < argc; ++i) {
			this->tokens.push_back(std::string(argv[i]));
		}
	}

	const std::string& getCmdOption(const std::string &option) const {
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

	bool get_option_argument(const CmdlineOption& cmdopt, std::string &value) {
		LOG_TRACE("cmdline::get_option_argument","%d", cmdopt.description.c_str());
		for (auto&& opt : cmdopt.options) {
			LOG_TRACE("cmdline::get_option_argument", "check %d", opt.c_str());
			if (cmdOptionExists(opt) == true) {
				LOG_TRACE("cmdline::get_option_argument", "cmdOptionExists %d == true", opt.c_str());
				std::string retval = getCmdOption(opt);
				LOG_TRACE("cmdline::get_option_argument", "getCmdOption %d == %s", opt.c_str(), retval.c_str());
				if (retval.length() > 0) {
					value = retval;
					return true;
				}
			}
		}
		LOG_TRACE("cmdline::get_option_argument", "not found");
		return false;
	}

	bool cmdOptionExists(const std::string &option) const
	{
		return std::find(this->tokens.begin(), this->tokens.end(), option) != this->tokens.end();
	}

	void addOption(CmdlineOption &cmdlineOption)
	{
		options.push_back(cmdlineOption);
	}
	bool isSet(CmdlineOption &cmdlineOption)
	{	
		std::ostringstream dbg_output;
		cmdlineOption.dump_options(dbg_output);
		LOG_TRACE("CmdlineOption::isSet","%s %s", cmdlineOption.description.c_str(), dbg_output.str());
		for (auto&& opt : cmdlineOption.options) {
			if (cmdOptionExists(opt) == true) {
				return true;
			}
		}
		return false;
	}
	bool invalidToken(){
		for (auto&& token : tokens){
			if (cmdlineOptionValid(token) == false) {
				return true;
			}
		}
		return false;
	}


	void dump_tokens(std::ostringstream& dbg_output) {
		for (auto&& token : tokens) {
			dbg_output << token.c_str() << std::endl;
		}
	}
private:
	bool cmdlineOptionValid(std::string opt)
	{
		for (auto&& elem : options){
			if (elem.isValid(opt)) {
				return true;
			}
		}
		return false;
	}
	std::vector <std::string> tokens;
	std::vector<CmdlineOption> options;
};

class CmdLineUtil
{
public:
	static CmdLineUtil* getInstance();

	void initializeCmdlineParser(int argc, char **argv);
	CmdlineParser *getInputParser() { return &inputParser; };

	void printTitle();

private:
	CmdLineUtil() {};

	CmdlineParser inputParser;
	static CmdLineUtil *instance;
};

#endif //__CMDLINE_H__