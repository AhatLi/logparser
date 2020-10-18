#include "DummyServer.h"
#include "HTTPMessage.h"


int LogProcesser::start() 
{
	sMsg.setHeaderCode("200");
	fMsg.setHeaderCode("400");

	//입력 대기
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

int LogProcesser::client_connect(int client_sock, InReqItem reqitem)
{
	std::string rBody = "";
	char buf[1024+1];
	char header[1024];
	HTTPMessage message;

	memset(buf, 0, 1024 + 1);
	memset(header, 0, 1024);

	//소켓에서 전송된 데이터 입력받음
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

	if (ret == -1)
	{
#ifdef _WIN32
		int err = WSAGetLastError();
		AhatLogger::ERR(CODE, "WSAGetLastError CODE = %d", err);
#elif __linux__   
#endif
	}

	std::stringstream ss(reqitem.in_req_port);
	int port;
	ss >> port;

	//올바른 HTTP 요청인지 확인하기 위해 body 확인
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

	//빠른 데이터 처리를 위해 HTTP 요청인지만 확인 후 응답을 보냄
	send(client_sock, sMsg.getMessage().c_str(), sMsg.getMessage().length(), 0);
	closeOsSocket(client_sock);
	strncat(header, rBody.substr(0, rBody.find("\r\n\r\n")).c_str(), rBody.find("\r\n\r\n"));
	rBody = rBody.substr(rBody.find("\r\n\r\n") + 4);

	makeResult(header, rBody, port, message, reqitem);
    AhatLogger::IN_REQ_DEBUG(CODE, reqitem, sMsg.getMessage());

	return 0;
}

std::string LogProcesser::makeResult(char* header, std::string body, int port, HTTPMessage message, InReqItem& reqitem)
{
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
	
	tok = strtok_all(NULL, "? \n", &saveptr1);
	if( !tok )
	{
		message.setHeaderCode("400");
		return message.getMessage();
    }

	std::string url =  tok;
	reqitem.in_req_url = std::string(tok);
	url = url.substr(1);

	//URL이 테이블 명이므로 비어있을 경우 실패
	if (url.empty())
	{
		message.setHeaderCode("400");
		return message.getMessage();
	}

	tok = strtok_all(NULL, "? \n/", &saveptr1);
	std::istringstream ss(body);
	std::string line;

	std::string sql = "";

	//여러 건의 데이터를 줄바꿈으로 전송받기 위해 처리하는 부분
	while (std::getline(ss, line, '\n'))
	{
		if (line.empty())
			continue;

		sql = parser.parsing(line.c_str(), url);

		//DBProcesser 로 파싱된 데이터 전송
		if (!dbp->Enqueue(url, sql))
		{
			message.setHeaderCode("400");
			return message.getMessage();
		}
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
	
	result = sMsg.getMessage();
	return "";
}

void LogProcesser::Enqueue(int client_sock, InReqItem reqitem)
{
	std::pair<int, InReqItem> p;
	p.first = client_sock;
	p.second = reqitem;

	q.push(p);
}

std::pair<int, InReqItem> LogProcesser::Dequeue()
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