#include "DBProcesser.h"

using namespace std;
//////////////////////////

DBProcesser::DBProcesser()
{
    config.readConfig("./LogParser");
    tSize = 0;

    for (auto it = config.configmap.begin(); it != config.configmap.end(); it++)
    {
        if (it->first.find("ColumnName") != std::string::npos)
        {
            std::string tname = it->first.substr(0, it->first.find("_ColumnName"));
            std::istringstream ss(it->second);
            std::string line;
            std::string values = "(";
            std::vector<int> v;
            tSize++;

            tmap[tname] = std::make_shared<std::vector<std::string> >();

            while (std::getline(ss, line, ','))
            {
                if (line.compare("X") != 0)
                {
                    values += line;
                    values += ", ";
                }
            }

            values = values.substr(0, values.length() - 2);
            values += ")";

            cmap[tname] = values;
        }
    }

    std::string Name = config.getConfig("ODBC_Name");
    std::string ID = config.getConfig("ODBC_ID");
    std::string PW = config.getConfig("ODBC_PW");

    ODBC_Name = (SQLCHAR*)Name.c_str();
    ODBC_ID = (SQLCHAR*)ID.c_str();
    ODBC_PW = (SQLCHAR*)PW.c_str();

    if (!DBConnect())
    {
        ErrorMSG();
    }
}

void DBProcesser::start()
{
    while (1)
    {
        for (auto it = tmap.begin(); it != tmap.end(); it++)
        {
            if (!it->second->empty())
            {
                mutex.lock();
                auto qq = it->second;
                it->second = std::make_shared<std::vector<std::string> >();
                mutex.unlock();

                excuteSQL(qq, it->first);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return;
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
/*
// 해당되는 쿼리 구문 실행 및 출력
bool DBProcesser::DBExcuteSQL(std::string values, std::string tName)
{
    RETCODE retcode;
    InDBtem item;
    std::string sql = "INSERT INTO ";
    sql += tName;
    sql += cmap[tName];
    sql += " VALUES ";
    sql += values;

    item.db_req_body = sql;

    std::wstring wstr = L"hello world";

    retcode = SQLExecDirect(hStmt, (SQLCHAR*)sql.c_str(), SQL_NTS);
    if ((retcode != SQL_SUCCESS) && (retcode != SQL_NEED_DATA) && (retcode != SQL_SUCCESS_WITH_INFO)) 
    {
        SQLINTEGER errNo;
        SQLSMALLINT msgLength;
        SQLCHAR errMsg[1024];

        if (SQL_SUCCESS == SQLError(hEnv, hDbc, hStmt, NULL, &errNo, errMsg, 1024, &msgLength))
        {
            printf(" ERR_-%ld : %s\n", errNo, errMsg);
            AhatLogger::DB_ERR_DEBUG(CODE, item, std::string((char*)errMsg));
            return false;
        }
    }
    AhatLogger::DB_DEBUG(CODE, item, "true");

    return true;
}
*/

// 해당되는 쿼리 구문 실행 및 출력
bool DBProcesser::excuteSQL(std::shared_ptr<std::vector<std::string> > v, std::string tName)
{
    std::string values = "";
    for (auto it = v->begin(); it != v->end(); it++)
    {
        values += it->c_str();
        values += ", ";
    }
    values = values.substr(0, values.length() - 2);

    RETCODE retcode;
    InDBtem item;
    std::string sql = "INSERT INTO ";
    sql += tName;
    sql += cmap[tName];
    sql += " VALUES ";
    sql += values;

    item.db_req_body = sql;

    std::wstring wstr = L"hello world";

    retcode = SQLExecDirect(hStmt, (SQLCHAR*)sql.c_str(), SQL_NTS);
    if ((retcode != SQL_SUCCESS) && (retcode != SQL_NEED_DATA) && (retcode != SQL_SUCCESS_WITH_INFO))
    {
        SQLINTEGER errNo;
        SQLSMALLINT msgLength;
        SQLCHAR errMsg[1024];

        if (SQL_SUCCESS == SQLError(hEnv, hDbc, hStmt, NULL, &errNo, errMsg, 1024, &msgLength))
        {
            printf(" ERR_-%ld : %s\n", errNo, errMsg);
            AhatLogger::DB_ERR_DEBUG(CODE, item, std::string((char*)errMsg));
            return false;
        }
    }
    AhatLogger::DB_DEBUG(CODE, item, "true");

    return true;
}