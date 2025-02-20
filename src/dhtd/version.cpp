//==============================================================================
//
//  version.cpp
//
//==============================================================================
//  automatically generated on Thursday, February 20, 2025 17:28
//==============================================================================

#include "stdafx.h"
#include <string.h>
#include "version.h"

#ifdef _RELEASE
unsigned int dhtd::version::major  = 1;
unsigned int dhtd::version::minor  = 2;
unsigned int dhtd::version::build  = 0;
unsigned int dhtd::version::rev    = release;
std::string  dhtd::version::sha    = "main";
std::string  dhtd::version::branch = "34936ddf";
#else
unsigned int dhtd::version::major  = 1;
unsigned int dhtd::version::minor  = 2;
unsigned int dhtd::version::build  = 0;
unsigned int dhtd::version::rev    = 56;
std::string  dhtd::version::sha    = "main";
std::string  dhtd::version::branch = "34936ddf";
#endif // _RELEASE
