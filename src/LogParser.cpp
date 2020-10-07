#include "LogParser.h"

logParser::logParser()
{
	config.readConfig("./LogParser");
}

std::string logParser::parsing(const char* msg, std::string tName)
{
	tName += "_";
	std::string tmp = tName + "LogScript";
	std::string logScript = config.getConfig(tmp);

	tmp = tName + "cName";
	std::string cName = config.getConfig(tmp);

	tmp = tName + "logNum";
	int logNum = config.getConfigInt(tmp);

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
	sscanf(msg, logScript.c_str(), p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15], p[16], p[17], p[18], p[19], p[20], p[21]
									, p[22], p[23], p[24], p[25], p[26], p[27], p[28], p[29], p[30], p[31], p[32], p[33], p[34], p[35], p[36], p[37], p[38], p[39]);

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