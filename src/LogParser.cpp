#include "LogParser.h"

logParser::logParser()
{
	config.readConfig("./LogParser");

	logScript = config.getConfig("logScript");
	cName = config.getConfig("cName");
	tName = config.getConfig("tName");
	logNum = config.getConfigInt("logNum");
}

std::string logParser::parsing(const char* msg)
{
	int count = 0;

	std::string sql = "INSERT INTO ";
	sql += tName;
	sql += "(";

	std::istringstream ss(cName);
	std::string line;

	std::vector<int> v;
	while (std::getline(ss, line, ','))
	{
		if (line.compare("X") != 0)
		{
			sql += line;
			sql += ", ";
			v.push_back(count);
		}
		count++;
	}
	sql = sql.substr(0, sql.length()-2);
	sql += ") VALUES (";

	char** p = new char* [logNum];
	for (int i = 0; i < logNum; i++)
	{
		p[i] = new char[256];
		memset(p[i], 0, 256);
	}
	sscanf(msg, logScript.c_str(), p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);

	for (int i = 0; i < v.size(); i++)
	{
		sql += "\"";
		sql += p[v[i]];
		sql += "\"";
		sql += ", ";
	}
	sql = sql.substr(0, sql.length() - 2);
	sql += ")";

	for (int i = 0; i < logNum; i++)
	{
		delete p[i];
	}
	delete[] p;

	return sql;
}