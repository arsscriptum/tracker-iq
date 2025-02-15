//==============================================================================
//
//  version.cpp
//
//==============================================================================
//  automatically generated on Saturday, February 15, 2025 3:06
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
std::string  dhtd::version::branch = "78ba5ad4";
#else
unsigned int dhtd::version::major  = 1;
unsigned int dhtd::version::minor  = 2;
unsigned int dhtd::version::build  = 0;
unsigned int dhtd::version::rev    = 32;
std::string  dhtd::version::sha    = "main";
std::string  dhtd::version::branch = "78ba5ad4";
#endif // _RELEASE
