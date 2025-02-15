//==============================================================================
//
//  cmdline_opt_values.cpp
//
//============================================================================
// Copyright (C)  Guillaume Plante <codegp@icloud.com>
//==============================================================================

ifndef __CMDLINE_OPT_VALUES_H__
#define __CMDLINE_OPT_VALUES_H__

#include <vector>
#include <string>

struct SCmdlineOptValues {
    std::vector<std::string> options;
    std::string description;
    std::string uid;

    // Constructor to initialize all fields at once.
    SCmdlineOptValues(const std::vector<std::string>& opts, 
                        const std::string& desc, 
                        const std::string& id)
        : options(opts), description(desc), uid(id) {}
};

#endif // __CMDLINE_OPT_VALUES_H__
