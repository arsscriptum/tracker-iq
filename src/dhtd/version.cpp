//==============================================================================
//
//  version.cpp
//
//==============================================================================
//  automatically generated on Feb 14, 2025 9:15:50 PM
//==============================================================================

#include "stdafx.h"
#include <string.h>
#include "version.h"

#ifdef _RELEASE
unsigned int dhtd::version::major  = 1;
unsigned int dhtd::version::minor  = 2;
unsigned int dhtd::version::build  = 0;
unsigned int dhtd::version::rev    = release;
std::string  dhtd::version::sha    = dev;
std::string  dhtd::version::branch = 044847e2;
#else
unsigned int dhtd::version::major  = 1;
unsigned int dhtd::version::minor  = 2;
unsigned int dhtd::version::build  = 0;
unsigned int dhtd::version::rev    = 15;
std::string  dhtd::version::sha    = dev;
std::string  dhtd::version::branch = 044847e2;
#endif // _RELEASE
