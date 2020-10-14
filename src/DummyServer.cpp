#include "DummyServer.h"
#include "HTTPMessage.h"


int DummyServer::start() 
{
	sMsg.setHeaderCode("200");
	fMsg.setHeaderCode("400");

	while (1)
	{
		if (!q.empty())
		{
			AhatLogger::INFO(CODE, "test 1 %s", getCurTime().c_str());
			auto data = Dequeue();
			client_connect(data.first, data.second);
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	return 0;
}

int DummyServer::client_connect(int client_sock, InReqItem reqitem)
{
	AhatLogger::INFO(CODE, "test 2 %s", getCurTime().c_str());
	std::string rBody = "";
	char buf[1024+1];
	char header[1024];
	HTTPMessage message;

	memset(buf, 0, 1024 + 1);
	memset(header, 0, 1024);
	AhatLogger::INFO(CODE, "test 3 %s", getCurTime().c_str());
	int ret = recv(client_sock, buf, 1024, 0);
	AhatLogger::INFO(CODE, "test 4 %s", getCurTime().c_str());
	while (1)
	{
		rBody += buf;
		if (ret < 1024)
		{
			break;
		}

		memset(buf, 0, 1024 + 1);
		ret = recv(client_sock, buf, 1024, 0);
	}
	AhatLogger::INFO(CODE, "test 5 %s", getCurTime().c_str());

	if (ret == -1)
	{
#ifdef _WIN32
		int err = WSAGetLastError();
		AhatLogger::ERR(CODE, "WSAGetLastError CODE = %d", err);
#elif __linux__   
#endif
	}
	AhatLogger::INFO(CODE, "test 6 %s", getCurTime().c_str());

	std::stringstream ss(reqitem.in_req_port);
	int port;
	ss >> port;

	reqitem.in_req_body = rBody;
	if (rBody.empty() || rBody.find("\r\n\r\n") == std::string::npos)
	{
		ret = send(client_sock, fMsg.getMessage().c_str(), fMsg.getMessage().length(), 0);

		if (ret == -1)
		{
#ifdef _WIN32
			int err = WSAGetLastError();
			AhatLogger::ERR(CODE, "WSAGetLastError CODE = %d", err);
#elif __linux__   
#endif
		}

		closeOsSocket(client_sock);
		AhatLogger::IN_REQ_ERR_DEBUG(CODE, reqitem, fMsg.getMessage());
		return 0;
	}
	AhatLogger::INFO(CODE, "test 7 %s", getCurTime().c_str());

	send(client_sock, sMsg.getMessage().c_str(), sMsg.getMessage().length(), 0);
	closeOsSocket(client_sock);
	AhatLogger::INFO(CODE, "test 8 %s", getCurTime().c_str());
	strncat(header, rBody.substr(0, rBody.find("\r\n\r\n")).c_str(), rBody.find("\r\n\r\n"));
	rBody = rBody.substr(rBody.find("\r\n\r\n") + 4);

	AhatLogger::INFO(CODE, "test 9 %s", getCurTime().c_str());
	makeResult(header, rBody, port, message, reqitem);
    AhatLogger::IN_REQ_DEBUG(CODE, reqitem, sMsg.getMessage());
	AhatLogger::INFO(CODE, "test 15 %s", getCurTime().c_str());

	return 0;
}

std::string DummyServer::makeResult(char* header, std::string body, int port, HTTPMessage message, InReqItem& reqitem)
{
	AhatLogger::INFO(CODE, "test 10 %s", getCurTime().c_str());
	std::string result;

	char* saveptr1;
	std::string pro;
	std::string api;
	std::string value;

	char* tok;
	bool isHttp = false;
	tok = strtok_all(header, " ", &saveptr1);

	if (!tok)
	{
		message.setHeaderCode("400");
		return message.getMessage();
	}
	pro = tok;
	
	if	//HTTP 프로토콜인지를 확인함 현재는 사용되지 않음
	(
	//	pro.compare("GET") == 0 ||
		!pro.compare("POST") == 0// ||
	//	pro.compare("HEAD") == 0 ||
	//	pro.compare("PUT") == 0 ||
	//	pro.compare("DELETE") == 0 ||
	//	pro.compare("OPTIONS") == 0 ||
	//	pro.compare("TRACE") == 0 ||
	//	pro.compare("CONNECT") == 0
	)
	{
		message.setHeaderCode("400");
		return message.getMessage();
	}
	AhatLogger::INFO(CODE, "test 11 %s", getCurTime().c_str());
	
	tok = strtok_all(NULL, "? \n", &saveptr1);
	if( !tok )
	{
		message.setHeaderCode("400");
		return message.getMessage();
    }

	std::string url =  tok;
	reqitem.in_req_url = std::string(tok);
	url = url.substr(1);

	if (url.empty())
	{
		message.setHeaderCode("400");
		return message.getMessage();
	}
	AhatLogger::INFO(CODE, "test 12 %s", getCurTime().c_str());

	tok = strtok_all(NULL, "? \n/", &saveptr1);
	std::istringstream ss(body);
	std::string line;

	std::string sql = "";
	AhatLogger::INFO(CODE, "test 13 %s", getCurTime().c_str());
	while (std::getline(ss, line, '\n'))
	{
		AhatLogger::INFO(CODE, "test 13-1 %s", getCurTime().c_str());
		if (line.empty())
			continue;

		AhatLogger::INFO(CODE, "test 13-2 %s", getCurTime().c_str());
		sql = parser.parsing(line.c_str(), url);
		AhatLogger::INFO(CODE, "test 13-3 %s", getCurTime().c_str());
	//	dbp.DBExcuteSQL(sql, url);
		AhatLogger::INFO(CODE, "test 13-4 %s", getCurTime().c_str());
		/*
		sql += parser.parsing(line.c_str());
		sql += ";\n";
		c++;

		if (c >= 100)
		{
			//table name : url
			sql = "";
			c = 0;
		}
		*/
	}
	AhatLogger::INFO(CODE, "test 14 %s", getCurTime().c_str());
	
	result = sMsg.getMessage();
	return "";
}

void DummyServer::Enqueue(int client_sock, InReqItem reqitem)
{
	std::pair<int, InReqItem> p;
	p.first = client_sock;
	p.second = reqitem;

	q.push(p);
}

std::pair<int, InReqItem> DummyServer::Dequeue()
{
	auto socket = q.front();
	q.pop();

	return socket;
}

int closeOsSocket(int socket)
{
#ifdef _WIN32
	return closesocket(socket);
#elif __linux__
	return close(socket);
#endif
}

char* strtok_all(char* _String, const char* _Delimiter, char** _Context)
{
#ifdef _WIN32
	return strtok_s(_String, _Delimiter, _Context);
#elif _WIN64

#elif __linux__
	return strtok_r(_String, _Delimiter, _Context);
#endif
}

std::string trim(std::string str)
{
	int n;
	n = str.find_first_not_of(" \t");
	if (n != std::string::npos)
	{
		str.replace(0, n, "");
	}
	n = str.find_last_not_of(" \t");
	if (n != std::string::npos)
	{
		str.replace(n + 1, str.length() - n, "");
	}
	return str;
}