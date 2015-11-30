/************************************************************************/
/*
create time:	2015/6/3
athor:			葛飞跃
discribe:		epoll响应器实现
*/
/************************************************************************/
#include <util/profile_test.h>
#include <net/tcpserver_impl.h>
#include <net/tcpclient_impl.h>
#include <net/reactor_epoll.h>

#ifdef HAVE_EPOLL
#include <netinet/tcp.h>

#define READ_EVENT_SIZE 10
#define WRITE_EVENT_SIZE 10

namespace fyreactor
{
	CReactor_Epoll::CReactor_Epoll(CTCPServerImpl* server, EReactorType type, handle_t handle, \
		std::mutex& mutexSendBuf, \
		std::unordered_map<socket_t, CBuffer>&	mapSendBuf, \
		std::mutex&	mutexEpoll)
		: m_pServer (server)
		, m_pClient(NULL)
		, m_iHandle(handle)
		, m_bRun(false)
		, m_eType (type)
		, m_iListenId(-1)
		, m_mutexSendBuf(mutexSendBuf)
		, m_mapSendBuf(mapSendBuf)
		, m_mutexEpoll(mutexEpoll)
	{
	}

	CReactor_Epoll::CReactor_Epoll(CTCPClientImpl* client, EReactorType type, handle_t handle, \
		std::mutex& mutexSendBuf, \
		std::unordered_map<socket_t, CBuffer>&	mapSendBuf, \
		std::mutex&	mutexEpoll)
		: m_pServer (NULL)
		, m_pClient(client)
		, m_iHandle(handle)
		, m_bRun(false)
		, m_eType (type)
		, m_iListenId(-1)
		, m_mutexSendBuf(mutexSendBuf)
		, m_mapSendBuf(mapSendBuf)
		, m_mutexEpoll(mutexEpoll)
	{
	}

	CReactor_Epoll::~CReactor_Epoll()
	{
	}	

	bool CReactor_Epoll::AddEvent(socket_t sockId, uint32_t e)
	{
		struct epoll_event ev;
		memset(&ev, 0, sizeof(ev));

		ev.events = this->ConvertEventMask(e);
		ev.data.fd = sockId;

		if (epoll_ctl(m_iHandle, EPOLL_CTL_ADD, sockId, &ev) < 0)
		{
			printf("epoll_ctl EPOLL_CTL_ADD error!");
			return false;
		}

		return true;
	}

	bool CReactor_Epoll::CtlEvent(socket_t sockId, uint32_t e)
	{
		struct epoll_event ev;
		memset(&ev, 0, sizeof(ev));

		ev.events = this->ConvertEventMask(e);
		ev.data.fd = sockId;

		if (epoll_ctl(m_iHandle, EPOLL_CTL_MOD, sockId, &ev) < 0)
		{
			printf("epoll_ctl EPOLL_CTL_MOD error, errno %d.", errno);
			return false;
		}

		return true;
	}

	bool CReactor_Epoll::DelEvent(socket_t sockId)
	{
		int result = epoll_ctl(m_iHandle, EPOLL_CTL_DEL, sockId, 0);
		if(result < 0)
		{
			//printf("epoll_ctl remove error %d", errno);
			return false;
		}

		return true;
	}

	void CReactor_Epoll::LoopAccept(int32 timeout)
	{
		int32_t result;
		socket_t newSock = -1;

		while (m_bRun)
		{
			result = epoll_wait(m_iHandle, m_aEvents, MAX_EVENT_SIZE, timeout < 0 ? INFINITE : timeout);

			if (result <  0)
			{
				if (errno == EINTR)
				{
					continue;
				}
				else
				{
					printf("epoll_wait end,errno=%d", errno);
					break;;
				}
			}

			if (result == 0)
			{
				continue;
			}

			for (int32_t i = 0; i<result; i++)
			{
				if (m_aEvents[i].data.fd == m_iListenId)
				{
					do
					{
						newSock = DoAccept();
						if (newSock != -1)
						{
							//通知tcpserver
							m_pServer->OnAccept(newSock);
						}
						else
							break;
					}while(true);

					//CtlEvent(m_iListenId, EVENT_READ);
				}
			}
		}
	}

	void CReactor_Epoll::LoopRead(int32 timeout)
	{
		char* buf1 = new char[MAX_MESSAGE_LEGNTH];
		int32_t result;
		int readLen;

		while (m_bRun)
		{
			{
				//std::unique_lock<std::mutex> lock (m_mutexEpoll);
				//m_mutexEpoll.lock();
				result = epoll_wait(m_iHandle, m_aEvents, READ_EVENT_SIZE, timeout < 0 ? INFINITE : timeout);

				if (result <  0)
				{
					if (errno == EINTR)
					{
						//m_mutexEpoll.unlock();
						continue;
					}
					else
					{
						printf("epoll_wait end,errno=%d", errno);
						//m_mutexEpoll.unlock();
						break;
					}
				}

				if (result == 0)
				{
					//m_mutexEpoll.unlock();
					continue;
				}

				for (int32_t i = 0; i<result; i++)
				{
					if (m_aEvents[i].events & EPOLLIN)
					{
						do {
							readLen = Recv(m_aEvents[i].data.fd, buf1);

							//m_mutexEpoll.unlock();
							if (readLen > 0)
							{							
								if (m_pServer != NULL)
									m_pServer->OnMessage(m_aEvents[i].data.fd, buf1, readLen);
								else if (m_pClient != NULL)
									m_pClient->OnMessage(m_aEvents[i].data.fd, buf1, readLen);
							}
							else if (readLen == -1)
							{
								if (m_pServer != NULL)
									m_pServer->OnClose(m_aEvents[i].data.fd);
								else if (m_pClient != NULL)
									m_pClient->OnClose(m_aEvents[i].data.fd);
							}

							//m_mutexEpoll.lock();
						}while (readLen == MAX_MESSAGE_LEGNTH);
					}				
				}

				//m_mutexEpoll.unlock();
			}
		}

		delete []buf1;
	}

	void CReactor_Epoll::LoopWrite(int32 timeout)
	{
		char* buf1 = new char[MAX_MESSAGE_LEGNTH];
		int32_t result;
		socket_t sockId;
		int sendLen;
		const char* msg;
		uint32_t len;

		while (m_bRun)
		{
			{
				//std::unique_lock<std::mutex> lock (m_mutexEpoll);
				result = epoll_wait(m_iHandle, m_aEvents, READ_EVENT_SIZE, timeout < 0 ? INFINITE : timeout);
			}

			if (result <  0)
			{
				if (errno == EINTR)
				{
					continue;
				}
				else
				{
					printf("epoll_wait end,errno=%d", errno);
					break;
				}
			}

			if (result == 0)
			{
				continue;
			}

			for (int32_t i = 0; i<result; i++)
			{
				if (m_aEvents[i].events & EPOLLOUT)
				{
					sockId = m_aEvents[i].data.fd;

					{
						std::unique_lock<std::mutex> lock(m_mutexSendBuf);
						msg = m_mapSendBuf[sockId].PopBuf(len);
						memcpy (buf1, msg, len);
					}
					if (len > 0)
					{
						sendLen = Send(sockId, buf1, len);
						if (sendLen < 0)
						{
							if (m_pServer != NULL)
								m_pServer->OnClose(sockId);
							else if (m_pClient != NULL)
								m_pClient->OnClose(sockId);

							break;
						}
					}
				}					
			}
		}

		delete []buf1;
	}

	void CReactor_Epoll::Loop(int32 timeout)
	{
		m_bRun = true;

		//1. 接收连接
		if (m_eType == REACTOR_LISTEN)
		{
			LoopAccept(timeout);
		}
		//2. 读
		else if (m_eType == REACTOR_READ)
		{
			LoopRead(timeout);
		}
		//3. 写
		else if (m_eType == REACTOR_WRITE)
		{
			LoopWrite(timeout);
		}
	}

	uint32_t CReactor_Epoll::ConvertEventMask(uint32_t e)
	{
		uint32_t op = 0;

		switch (e)
		{
		case EVENT_OPEN:
			op = EPOLLIN;
			break;
		case EVENT_READ:
			op = EPOLLIN | EPOLLET;
			break;
		case EVENT_WRITE:
			op = EPOLLOUT | EPOLLONESHOT | EPOLLET;
			break;
		default:
			break;
		}

		return op;
	}

	void CReactor_Epoll::ReadySendMessage(socket_t sockId, const char* message, uint32_t len)
	{
		int32_t res = -1;
		int retryNum = 10;

		do
		{
			{
				std::unique_lock<std::mutex> lock(m_mutexSendBuf);
				res = m_mapSendBuf[sockId].AddBuf(message, len);
			}

			if (!CtlEvent (sockId, EVENT_WRITE))
			{
				//if (errno == ENOENT)
				{
					//此时表明socket已经关闭，并且执行了DelEvent
					std::unique_lock<std::mutex> lock(m_mutexSendBuf);
					m_mapSendBuf.erase(sockId);
					return;
				}
			}

			if (res == -1)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(3));
			}
			else
			{
				return;
			}

		}while (--retryNum >= 0);

		printf("ReadySendMessage failed \n");		
	}

	void CReactor_Epoll::OnClose(socket_t sockId)
	{
		if (m_eType == REACTOR_WRITE)
		{
			std::unique_lock<std::mutex> lock(m_mutexSendBuf);
			m_mapSendBuf.erase(sockId);
		}
	}

	socket_t CReactor_Epoll::CreateNewSocket()
	{
		socket_t sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		return sock;
	}

	bool CReactor_Epoll::InitSocket(socket_t sock)
	{
		if (sock == -1)
		{
			return false;
		}

		int i = 1;
		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&i, sizeof(int)) == -1)
		{
			printf("setsocketopt SO_REUSEADDR error");
			return false;
		}

		if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&i, sizeof(int)) == -1)
		{
			printf("setsocketopt TCP_NODELAY error");
			return false;
		}

		int sendMax = MAX_SEND_SIZE;
		if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&sendMax, sizeof(sendMax)) == -1)
		{
			return false;
		}

		if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&sendMax, sizeof(sendMax)) == -1)
		{
			return false;
		}

		{
			int keepalive = 1; // 开启keepalive属性
			int keepidle = 60; // 如该连接在60秒内没有任何数据往来,则进行探测
			int keepinterval = 5; // 探测时发包的时间间隔为5 秒
			int keepcount = 3; // 探测尝试的次数.如果第1次探测包就收到响应了,则后2次的不再发.
			setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive , sizeof(keepalive ));
			setsockopt(sock, SOL_TCP, TCP_KEEPIDLE, (void*)&keepidle , sizeof(keepidle ));
			setsockopt(sock, SOL_TCP, TCP_KEEPINTVL, (void *)&keepinterval , sizeof(keepinterval ));
			setsockopt(sock, SOL_TCP, TCP_KEEPCNT, (void *)&keepcount , sizeof(keepcount ));
		}

		int flags = fcntl(sock, F_GETFL, 0);
		if (flags == -1)
			return false;
		
		flags |= O_NONBLOCK;
		if (fcntl(sock, F_SETFL, flags) != 0)
			return false;

		return true;
	}

	socket_t CReactor_Epoll::DoAccept()
	{
		if (m_eType == REACTOR_LISTEN)
		{
			struct sockaddr_in client_address;
			socklen_t length = sizeof(struct sockaddr_in);
			socket_t socket = 0;

			socket = ::accept(m_iListenId, (sockaddr *)&client_address, &length);
			if (socket < 0 || socket == 0)
			{
				return -1;
			}
			if (!InitSocket(socket))
			{
				return -1;
			}

			return socket;
		}

		return -1;
	}

	bool CReactor_Epoll::Listen(const std::string& ip, int port)
	{
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(ip.c_str());
		addr.sin_port = htons(port);

		socket_t listen_socket = CreateNewSocket();
		if (listen_socket == -1)
			return false;

		if (InitSocket(listen_socket) == false)
			return false;

		if (::bind(listen_socket, (sockaddr *)&addr, sizeof(addr)) == -1)
		{
			printf("socket bind faild, errno %d.", errno);
			return false;
		}

		//listen的第二个参数即backlog,测试发现数值+1代表呼入连接请求队列长度，即目前队列长度为9
		//参见tcp/ip协议 18.11.4
		if (::listen(listen_socket, 8) == -1)
		{
			printf("socket listen faild, errno %d.", errno);
			return false;
		}

		m_iListenId = listen_socket;
		if (!AddEvent(m_iListenId, EVENT_OPEN))
			return false;

		return true;
	}

	socket_t CReactor_Epoll::Connect(const std::string& ip, int port)
	{
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(ip.c_str());
		addr.sin_port = htons(port);

		socket_t sockId = CreateNewSocket();
		if (!InitSocket(sockId))
			return -1;

		int retryNum = 3;
		do
		{
			if (::connect(sockId, (sockaddr*)&addr, sizeof(sockaddr)) != SOCKET_ERROR)
			{
				return sockId;
			}
		}while(--retryNum >= 0);
		
		return -1;
	}

	int CReactor_Epoll::Recv(socket_t sockId, char* buf)
	{
		int result = 0;
		int hasRead = 0;

		while (true)
		{
			result = ::recv(sockId, buf + hasRead, MAX_MESSAGE_LEGNTH - hasRead, 0);
			if (result == 0)
			{
				::close(sockId);
				return -1;
			}

			if (result < 0)
			{
				if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
				{
					break;
				}

				printf("socket(%d) recv error, errno %d.", sockId, errno);
				::close(sockId);
				return -1;
			}

			hasRead += result;

			if (hasRead == MAX_MESSAGE_LEGNTH)
				break;
		}

		return hasRead;
	}

	int CReactor_Epoll::Send(socket_t sockId, const char* message, uint32_t len)
	{
		int32 now_size = len;
		int32 result = 0;
		int32 send_len = 0;

		while (now_size > 0)
		{
			if (now_size < MAX_MESSAGE_LEGNTH)
				send_len = now_size;
			else
				send_len = MAX_MESSAGE_LEGNTH;

			result = ::send(sockId, (message + (len - now_size)), send_len, 0);
			if (result < now_size)
			{
				if (result == 0)
				{
					continue;
				}

				if (result < 0)
				{
					if (EAGAIN == errno || EWOULDBLOCK == errno || EINTR == errno)
					{
						continue;
					}
					else
					{
						printf("socket(%d) send error, errno %d.", sockId, errno);
						::close(sockId);
						return -1;
					}
				}
			}
			now_size -= result;
		}

		return len - now_size;
	}

}

#endif //HAVE_EPOLL
