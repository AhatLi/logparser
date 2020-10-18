#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <thread>
#include <stdio.h> 
#include <memory>

#ifdef _WIN32
#include <winsock2.h> 
#elif __linux__
#include <sys/epoll.h>
#define EPOLL_SIZE 20
#endif

#include "DummyServer.h"
#include "ahatlogger.h"
#include "LogParser.h"

int main(int argc, char *argv[]) 
{
#ifdef _WIN32
    unsigned int fd_num = 0;
#elif __linux__
    //리눅스 환경에서 코어덤프 남도록 설정
	struct rlimit lim;
	getrlimit(RLIMIT_CORE, &lim);
	lim.rlim_cur = lim.rlim_max;
	setrlimit(RLIMIT_CORE, &lim);
    unsigned int j;
#endif
    //소켓과 포트를 여러개 입력받을 수 있도록 포인터 배열로 생성
    int* listen_sock = new int[argc - 1];
    int* port = new int[argc - 1];
    int client_sock;
    unsigned int i, c = 0;
    int port_size = 0;
    std::vector<std::shared_ptr<LogProcesser> > threads;

    bool debug = false;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-g") == 0)
        {
            debug = true;
            break;
        }
    }

    //Debug 모드 감지
    if (debug)
    {
        AhatLogger::setting("", "LogParser", 0);
        AhatLogger::start();
        AhatLogger::DEBUG(CODE, "LogParser DEBUG MODE Start");
    }
    else
    {
        AhatLogger::setting("", "LogParser", 1);
        AhatLogger::start();
    }

    if (argc < 2)
    {
        AhatLogger::INFO(CODE, "insert port number");
        std::cout << "insert port number\n";
        AhatLogger::stop();
        return 0;
    }

    //입력받은 포트 개수만큼 포트 사이즈 개수 넣기
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-g") == 0)
        {
            continue;
        }

        std::stringstream str;
        str << argv[i];
        str >> port[port_size];

        if (port[port_size] < 0 || port[port_size] > 65535)
        {
            AhatLogger::ERR(CODE, "%d is bad request!", port);
            AhatLogger::stop();
            return 0;
        }
        AhatLogger::DEBUG(CODE, "Server port %d : %d", i, port[port_size]);

        port_size++;
    }

    //DBProcesser 스레드 생성
    std::shared_ptr<DBProcesser> dbp = std::make_shared<DBProcesser>();
    std::thread tt(&DBProcesser::start, dbp);
    tt.detach();

    //CPU의 스레드 수 -1 만큼 LogProcesser 스레드 생성 최소 1개
    int core = std::thread::hardware_concurrency();
    if (core < 2)
        core = 2;

    for (int i = 0; i < core - 1; i++)
    {
        auto server = std::make_shared<LogProcesser>();
        server->dbp = dbp;

        threads.push_back(server);
        std::thread t(&LogProcesser::start, server);
        t.detach();
    }

#ifdef _WIN32
    WSADATA wsaData;
    int addrlen;
    SOCKADDR_IN serveraddr, clientaddr;
    fd_set old_fds, new_fds;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cout << "WSAStartup Error\n";
        AhatLogger::ERR(CODE, "WSAStartup Error");
        AhatLogger::stop();
        return 0;
    }
    FD_ZERO(&new_fds);
#elif __linux__
    struct sockaddr_in serveraddr, clientaddr;
    socklen_t addrlen;

    int eventn;
    int epollfd;
    struct epoll_event ev, *events;

    events = (struct epoll_event*)malloc(sizeof(struct epoll_event) * EPOLL_SIZE);
    if ((epollfd = epoll_create(100)) == -1)
    {
        std::cout << "epoll_create Error!\n";
        AhatLogger::ERR(CODE, "epoll_create Error!");
        AhatLogger::stop();
    }
#endif
    AhatLogger::INFO(CODE, "LogParser start success");
    std::cout << "LogParser start success\n";
    
    //입력된 포트 수 만큼 소켓 생성
    for (int i = 0; i < port_size; i++)
    {
#ifdef _WIN32
        listen_sock[i] = socket(PF_INET, SOCK_STREAM, 0);
        if (listen_sock[i] == INVALID_SOCKET)
        {
            std::cout << "Socket Error!\n";
            AhatLogger::ERR(CODE, "Socket Error!");
            AhatLogger::stop();
            return 0;
        }
#elif __linux__
        struct sockaddr_in clientaddr;
        socklen_t addrlen;
        listen_sock[i] = socket(PF_INET, SOCK_STREAM, 0);
        if (listen_sock[i] < 0)
        {
            std::cout << "Socket Error!\n";
            AhatLogger::ERR(CODE, "Socket Error!");
            AhatLogger::stop();
            return 0;
        }
#endif
        memset((void*)&serveraddr, 0x00, sizeof(serveraddr));

        serveraddr.sin_family = AF_INET;
        serveraddr.sin_port = htons(port[i]);
        serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(listen_sock[i], (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1)
        {
            std::cout<< "socket error\n";
            AhatLogger::ERR(CODE, "%d port socket error", port[i]);
            AhatLogger::stop();
            return 0;
        }

        if(listen(listen_sock[i], SOMAXCONN) == -1)
        {
            std::cout << "listen error\n";
            AhatLogger::stop();
            return 0;
        }

#ifdef _WIN32
        FD_SET(listen_sock[i], &new_fds);
#elif __linux__    
        ev.events = EPOLLIN;
        ev.data.fd = listen_sock[i];
        epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock[i], &ev);
#endif
    }

    //멀티플렉싱 방식을 사용하여 여러개의 포트에서 accept 요청을 받음
    while (1)
    {
#ifdef _WIN32
        old_fds = new_fds;
        fd_num = select(0, &old_fds, NULL, NULL, NULL);
#elif __linux__    
        eventn = epoll_wait(epollfd, events, EPOLL_SIZE, -1);
#endif
        for (i = 0; i < port_size; i++)
        {
#ifdef _WIN32
            if (FD_ISSET(listen_sock[i], &old_fds))
            {
#elif __linux__    
            for (j = 0; j < eventn; j++)
            {
                if (events[j].data.fd == listen_sock[i])    // 듣기 소켓에서 이벤트가 발생함
                {
#endif
                addrlen = sizeof(clientaddr);
                client_sock = accept(listen_sock[i], (struct sockaddr*) & clientaddr, &addrlen);
                if (client_sock < 0)
                {
                    continue;
                }

                InReqItem reqitem(inet_ntoa(clientaddr.sin_addr), std::to_string(port[i]), "");
                AhatLogger::INFO(CODE, "Client Accept, %s, %d", inet_ntoa(clientaddr.sin_addr), port[i]);

                //http 요청을 받을 경우 LogProcesser 스레드의 큐에 입력
                threads[c++]->Enqueue(client_sock, reqitem);
                if (core <= c + 1)
                    c = 0;
#ifdef __linux__    
                }
#endif
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

#ifdef _WIN32
    WSACleanup();
#endif
    AhatLogger::stop();
    return 0;
}