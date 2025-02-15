
//==============================================================================
//
//     cmdline.cpp
//
//============================================================================
//  Copyright (C) Guilaume Plante 2020 <cybercastor@icloud.com>
//==============================================================================


#include "stdafx.h"
#include <cmdline.h>
#include "log.h"

CmdLineUtil* CmdLineUtil::instance = nullptr;

#define APP_NAME		"svcm"
#define APP_DESC		"Windows Service Manipulator"
#define EXECUTABLE_NAME	"svcm.exe"
#define APP_VERSION		"v1.0"
#define APP_COPYRIGHT	"Copyright(C) 2000 - 2021 Guillaume Plante"
#define APP_CONTACT		"codecastor - codecastor.github.io"


CmdLineUtil* CmdLineUtil::getInstance()
{
	if (instance == nullptr)
	{
		instance = new CmdLineUtil();
	}

	return instance;
}


void CmdLineUtil::initializeCmdlineParser(int argc,  char **argv)
{
	inputParser.reset(argc, argv); 
}

void CmdLineUtil::printTitle()
{
	std::cout << std::endl;
	std::cout << APP_NAME << " " << APP_VERSION << " - " << APP_DESC << std::endl;
	std::cout << APP_COPYRIGHT << std::endl;
	std::cout << APP_CONTACT << std::endl;
	std::cout << std::endl;
}
