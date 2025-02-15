//==============================================================================
//
//  version.cpp
//
//==============================================================================
//  automatically generated on Saturday, February 15, 2025 1:06
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
std::string  dhtd::version::branch = "164cd2ff";
#else
unsigned int dhtd::version::major  = 1;
unsigned int dhtd::version::minor  = 2;
unsigned int dhtd::version::build  = 0;
unsigned int dhtd::version::rev    = 10;
std::string  dhtd::version::sha    = "main";
std::string  dhtd::version::branch = "164cd2ff";
#endif // _RELEASE
