#include "LogParser.h"

logParser::logParser()
{
	config.readConfig("./LogParser");

	for (auto it = config.configmap.begin(); it != config.configmap.end(); it++) 
	{
		if (it->first.find("ColumnName") != std::string::npos)
		{			
			std::string tname = it->first.substr(0, it->first.find("_ColumnName"));
			std::istringstream ss(it->second);
			std::string line;
			std::vector<int> v;
			int count = 0;

			while (std::getline(ss, line, ','))
			{
				if (line.compare("X") != 0)
				{
					v.push_back(count);
				}
				count++;
			}

			cmap[tname] = v;
		}
	}
}

std::string logParser::parsing(const char* msg, std::string tName)
{
	AhatLogger::DEBUG(CODE, "msg : %s", msg);
	AhatLogger::DEBUG(CODE, "table name : %s", tName.c_str());
	std::string cfgTmp = tName;
	cfgTmp += "_";
	std::string tmp = cfgTmp + "LogScript";
	std::string logScript = config.getConfig(tmp);
	AhatLogger::DEBUG(CODE, "conf name : [%s] value : %s", tmp.c_str(), logScript.c_str());

	tmp = cfgTmp + "LogNum";
	int logNum = config.getConfigInt(tmp);
	AhatLogger::DEBUG(CODE, "conf name : [%s] value : %d", tmp.c_str(), logNum);

	int count = 0;

	std::string sql = "(";

	auto v = cmap[tName];

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

	AhatLogger::DEBUG(CODE, "sql : %s", sql.c_str());

	for (int i = 0; i < logNum; i++)
	{
		delete p[i];
	}
	delete[] p;

	return sql;
}