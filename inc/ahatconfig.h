#ifndef AHATCONFIG_H_
#define AHATCONFIG_H_

#include <iostream>
#include <sstream>
#include <map>

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <unistd.h>
#endif
#include <fstream>
#include <fcntl.h>

class AhatConfig
{
private:
	std::map<std::string, std::string> configmap;
	std::string trim(std::string str);

public:
//	bool readConfig(std::string path);
	bool readConfig(const char* path);

	std::string getConfig(std::string name);
	std::string getConfig(const char* name);
	int getConfigInt(std::string name);
	int getConfigInt(const char* name);

	bool setConfig(std::string name, std::string value);
	bool setConfig(const char* name, const char* value);
};

#endif
