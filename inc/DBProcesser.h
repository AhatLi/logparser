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

#include "ahatconfig.h"

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
public:
	DBProcesser();
	bool DBConnect();
	void DBDisConnect();
	bool DBExcuteSQL(std::string sql);
};

#endif
