#include "ahatconfig.h"
/*
bool AhatConfig::readConfig(std::string path)
{
	return readConfig(path.c_str());
}
*/
bool AhatConfig::readConfig(const char* path)
{
	std::string configPath = "";
	if(path == NULL)
	{
#ifdef _WIN32
		/*
		LPSTR tmp;
		int len = GetModuleFileName(NULL, tmp, MAX_PATH);
	//	std::wstring ws(tmp);
		std::string buf = tmp;
		buf = buf.substr(0, buf.find_last_of("."));
		*/
		std::string buf = "./";
#elif __linux__
		char buf[256];
		int len = readlink("/proc/self/exe", buf, 256);
		buf[len] = '\0';
#endif
		configPath += buf;
	}
	else
	{
		configPath += path;
	}
	configPath += ".cfg";

	std::string data = "";
	int num = 0;
	char buf[1024+1];

//#ifdef _WIN32
	std::ifstream inFile(configPath.c_str());

	if (inFile.fail())
	{
		std::cout << "Read Config Fail !! " << configPath.c_str() << "\n";
		return false;
	}

	while (!inFile.eof()) 
	{
		inFile.getline(buf, 1024);
		data += buf;
		data += "\n";
		
		std::string line = buf;

		if (line.find("//") != std::string::npos)
		{
			line = line.substr(0, line.find("//"));
		}
		if (line.find("#") != std::string::npos)
		{
			line = line.substr(0, line.find("#"));
		}
		if (line.find("=") != std::string::npos)
		{
			std::istringstream sss(line);
			std::string name = "";
			std::string value = "";

			std::getline(sss, name, '=');
			std::getline(sss, value, '=');
			configmap[trim(name)] = trim(value);
		}
	}
	inFile.close();
	/*
#elif __linux__
	int fd = open(configPath.c_str(), O_RDONLY);
	if (fd == -1)
	{
		std::cout << configPath.c_str() << " file not found!\n";
		return false;
	}
	while ((num = read(fd, buf, 128)) > 0)
	{
		buf[num + 1] = '\0';
		data += buf;
	}
	close(fd);
#endif

	std::istringstream ss(data);
	std::string line;

	while(std::getline(ss, line, '\n'))
	{
		if(line.find("//") != std::string::npos)
		{
			line = line.substr(0, line.find("//"));
		}
		if(line.find("#") != std::string::npos)
		{
			line = line.substr(0, line.find("#"));
		}
		if(line.find("=") != std::string::npos)
		{
			std::istringstream sss(line);
			std::string name = "";
			std::string value = "";

			std::getline(sss, name, '=');
			std::getline(sss, value, '=');
			configmap[trim(name)] = trim(value);
		}
	}
	*/
	return true;
}

std::string AhatConfig::trim(std::string str)
{
	int n; 
	n = str.find_first_not_of(" \t"); 
	if ( n != std::string::npos ) 
	{
		str.replace(0, n,""); 
	}
	n = str.find_last_not_of(" \t"); 
	if ( n != std::string::npos )
	{
		str.replace(n+1, str.length()-n,""); 
	}
	return str;
}

std::string AhatConfig::getConfig(std::string name)
{
	if(configmap[name] == std::string())
	{
		return std::string();
	}
	return configmap[name];
}

std::string AhatConfig::getConfig(const char* name)
{
	return getConfig(std::string(name));
}

int AhatConfig::getConfigInt(std::string name)
{
	if (configmap[name] == std::string())
	{
		return 0;
	}
	return atoi(configmap[name].c_str());
}

int AhatConfig::getConfigInt(const char* name)
{
	return getConfigInt(std::string(name));
}

bool AhatConfig::setConfig(std::string name, std::string value)
{
	configmap[name] = value;
	return true;
}

bool AhatConfig::setConfig(const char* name, const char* value)
{
	return setConfig(std::string(name), std::string(value));
}

