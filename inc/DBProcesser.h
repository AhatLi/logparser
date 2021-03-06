#ifndef __DB_PROCESSER_H__
#define __DB_PROCESSER_H__

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <unistd.h>
#endif
#include <iostream>
#include <sql.h>
#include <sqlext.h>
#include <vector>

#include "ahatconfig.h"
#include "ahatlogger.h"

class DBProcesser
{
	SQLHENV hEnv;
	SQLHDBC hDbc;
	SQLHSTMT hStmt;

	SQLCHAR* ODBC_Name;
	SQLCHAR* ODBC_ID;
	SQLCHAR* ODBC_PW;

	AhatConfig config;

	void Cleanup();
	void ErrorMSG();
	std::map< std::string, std::string > cmap;

	std::map<std::string, std::shared_ptr<std::vector<std::string> > > tmap;
	std::map<std::string, int > smap;
	int tSize;
	std::mutex mutex;
public:
	void start();
	DBProcesser();
	bool DBConnect();
	void DBDisConnect();
//	bool DBExcuteSQL(std::string sql, std::string tName);
	bool excuteSQL(std::shared_ptr<std::vector<std::string> > v, std::string tName);

	bool Enqueue(std::string tname, std::string value);
};

#endif
