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


enum class cmdlineOptTypes : uint8_t { Help, Verbose, NoBanner, Quiet, cfgfilePath };
using cmdOpT = cmdlineOptTypes;

struct SCmdlineOptValues {
    std::vector<std::string> options;
    std::string description;
    cmdlineOptTypes type;;

    // Constructor to initialize all fields at once.
    SCmdlineOptValues(const std::vector<std::string>& opts, 
                        const std::string& desc, 
                        cmdOpT id)
        : options(opts), description(desc), type(id) {}
};

#endif // __CMDLINE_OPT_VALUES_H__
