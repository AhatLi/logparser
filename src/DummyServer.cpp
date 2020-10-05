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
	std::string rBody = "";
	char buf[1024+1];
	HTTPMessage message;

	memset(buf, 0, 1024 + 1);
	int ret = recv(client_sock, buf, 1024, 0);
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

	std::string result = sMsg.getMessage();
	send(client_sock, result.c_str(), result.length(), 0);
	closeOsSocket(client_sock);

	std::stringstream ss(reqitem.in_req_port);
	int port;
	ss >> port;
	
	reqitem.in_req_body = std::string(buf);
	makeResult(buf, port, message, reqitem);
    AhatLogger::IN_REQ_DEBUG(CODE, reqitem, result);

	return 0;
}

std::string DummyServer::makeResult(char* msg, int port, HTTPMessage message, InReqItem& reqitem)
{
	std::string result;
	std::string header;
	std::string body;

	char* saveptr1;
	std::string pro;
	std::string api;
	std::string value;

	char* tok;
	bool isHttp = false;
	std::string bodyText = msg;
	tok = strtok_all(msg, " ", &saveptr1);

	if (!tok)
	{
		message.setHeaderCode("404");
		return message.getMessage();
	}
	pro = tok;
	
	if	//HTTP 프로토콜인지를 확인함 현재는 사용되지 않음
	(
		pro.compare("GET") == 0 ||
		pro.compare("POST") == 0 ||
		pro.compare("HEAD") == 0 ||
		pro.compare("PUT") == 0 ||
		pro.compare("DELETE") == 0 ||
		pro.compare("OPTIONS") == 0 ||
		pro.compare("TRACE") == 0 ||
		pro.compare("CONNECT") == 0
	)
	{
		isHttp = true;
	}
	
	tok = strtok_all(NULL, "? \n", &saveptr1);
	if( !tok )
	{
		message.setHeaderCode("404");
		return message.getMessage();
    }

	std::string path = "";	//API 주소
	path += tok;
	reqitem.in_req_url = std::string(tok);

	if (pro.compare("POST") == 0)
	{
		tok = strtok_all(NULL, "? \n/", &saveptr1);
		if (strcmp(tok, "HTTP") == 0)
		{
			bodyText = bodyText.substr(bodyText.find("\r\n\r\n") + 4);
		}
	}
	else
	{
		message.setHeaderCode("404");
		return message.getMessage();
	}

	std::string sql = parser.parsing(bodyText.c_str());
	dbp.DBExcuteSQL(sql);
	
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
	auto socket = q.back();
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