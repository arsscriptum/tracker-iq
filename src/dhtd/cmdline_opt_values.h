//==============================================================================
//
//  cmdline_opt_values.cpp
//
//============================================================================
// Copyright (C)  Guillaume Plante <codegp@icloud.com>
//==============================================================================

#ifndef __CMDLINE_OPT_VALUES_H__
#define __CMDLINE_OPT_VALUES_H__

#include <vector>
#include <string>

#ifndef UINT8_MAX
typedef unsigned char uint8_t;
#endif


enum class cmdlineOptTypes : uint8_t { Unknown, Help, Verbose, NoBanner, Quiet, cfgfilePath };
using cmdOpT = cmdlineOptTypes;

struct SCmdlineOptValues {
    std::vector<std::string> _options;
    std::string _description;
    cmdOpT _type;
    SCmdlineOptValues()
        : _options(), _description(""), _type(cmdOpT::Unknown) {}
    // Constructor to initialize all members.
    SCmdlineOptValues(const std::vector<std::string>& opts,
        const std::string& desc,
        cmdOpT id)
        : _options(opts), _description(desc), _type(id) {}
};

#endif // __CMDLINE_OPT_VALUES_H__
