#ifndef __DB_PROCESSER_H__
#define __DB_PROCESSER_H__

#include <windows.h>
#include <iostream>
#include <sql.h>
#include <sqlext.h>

class DBProcesser
{
public:
	DBProcesser();
	bool DBConnect();
	void DBDisConnect();
	bool DBExcuteSQL(std::string sql);
	void Cleanup();
	void ErrorMSG();

	SQLHENV hEnv;
	SQLHDBC hDbc;
	SQLHSTMT hStmt;

	SQLCHAR* ODBC_Name;
	SQLCHAR* ODBC_ID;
	SQLCHAR* ODBC_PW;
};

#endif