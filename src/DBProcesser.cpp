#include "DBProcesser.h"

using namespace std;
//////////////////////////

DBProcesser::DBProcesser()
{
    ODBC_Name = (SQLCHAR*)"mariadb";
    ODBC_ID = (SQLCHAR*)"user";
    ODBC_PW = (SQLCHAR*)"1qw2wa!!@@";

    if (!DBConnect())
    {
        ErrorMSG();
    }
}

void DBProcesser::ErrorMSG()
{
    SQLSMALLINT msg_len = 0;
    SQLCHAR sql_state[6], message[256];
    SQLINTEGER native_error = 0;

    SQLGetDiagRec(SQL_HANDLE_DBC, hDbc, 1, sql_state, &native_error, message, 256, &msg_len);
    std::cout << message << endl;

    exit(0);
}

// ODBC사용 핸들 할당 및 SQL접속 부분
bool DBProcesser::DBConnect()
{
    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) != SQL_SUCCESS)
        return false;

    if (SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER) != SQL_SUCCESS)
        return false;

    if (SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc) != SQL_SUCCESS)
        return false;

    if(SQLConnect(hDbc, ODBC_Name, SQL_NTS, ODBC_ID, SQL_NTS, ODBC_PW, SQL_NTS) != SQL_SUCCESS)
        return false;

    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) != SQL_SUCCESS)
        return false;

    return true;
}

void DBProcesser::Cleanup()
{
    if (hStmt != SQL_NULL_HSTMT)
    {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    }

    if (hDbc != SQL_NULL_HDBC) 
    {
        SQLDisconnect(hDbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    }

    if (hEnv != SQL_NULL_HENV)
    {
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
    }
}

// 접속 종료 및 핸들 반환
void DBProcesser::DBDisConnect()
{
    if (hStmt) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    if (hDbc) SQLDisconnect(hDbc);
    if (hDbc) SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    if (hEnv) SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}

// 해당되는 쿼리 구문 실행 및 출력
bool DBProcesser::DBExcuteSQL(std::string sql)
{
    RETCODE retcode;

    std::wstring wstr = L"hello world";

    retcode = SQLExecDirect(hStmt, (SQLCHAR*)sql.c_str(), SQL_NTS);
    if ((retcode != SQL_SUCCESS) && (retcode != SQL_NEED_DATA) && (retcode != SQL_SUCCESS_WITH_INFO)) 
    {
        SQLINTEGER errNo;
        SQLSMALLINT msgLength;
        SQLCHAR errMsg[1024];

        if (SQL_SUCCESS == SQLError(hEnv, hDbc, hStmt, NULL, &errNo, errMsg, 1024, &msgLength))
        {
            printf(" ERR_-%ld : %s?n", errNo, errMsg);
        }
    }

    return true;
}