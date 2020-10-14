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
#include "ahatlogger.h"

class logParser
{
	AhatConfig config;

	std::map< std::string, std::vector<int> > cmap;
public:
	logParser();
	std::string parsing(const char* msg, std::string tName);
};

#endif