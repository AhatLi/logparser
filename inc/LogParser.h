#ifndef __LOG_PARSER_H__
#define __LOG_PARSER_H__

#include <iostream>
#include <thread>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstdio>
#include <string>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <map>
#include <queue>

#include "ahatconfig.h"

class logParser
{
	AhatConfig config;

	std::string logScript;
	std::string cName;
	std::string tName;
	int logNum;

public:
	logParser();
	std::string parsing(const char* msg);
};

#endif