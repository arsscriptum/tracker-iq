//==============================================================================
//
//  version.cpp
//
//==============================================================================
//  automatically generated on Sunday, February 16, 2025 20:11
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
std::string  dhtd::version::branch = "c8d0d254";
#else
unsigned int dhtd::version::major  = 1;
unsigned int dhtd::version::minor  = 2;
unsigned int dhtd::version::build  = 0;
unsigned int dhtd::version::rev    = 37;
std::string  dhtd::version::sha    = "main";
std::string  dhtd::version::branch = "c8d0d254";
#endif // _RELEASE
