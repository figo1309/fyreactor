/************************************************************************/
/*
create time:	2015/6/3
athor:			葛飞跃
discribe:		epoll响应器实现
*/
/************************************************************************/
#include <net/tcpserver.h>
#include <net/tcpclient.h>
#include <net/reactor_epoll.h>

#ifdef HAVE_EPOLL

namespace fyreactor
{
	CReactor_Epoll::CReactor_Epoll(CTCPServer* server, EReactorType type)
		: m_pServer (server)
		, m_pClient(NULL)
		, m_bRun(false)
		, m_eType (type)
		, m_iListenId(-1)
	{
		m_iHandle = epoll_create(MAX_EVENT_SIZE);
		if (m_iHandle == -1)
		{
			printf("epoll_create() Error.");
		}
	}

	CReactor_Epoll::CReactor_Epoll(CTCPClient* client, EReactorType type)
		: m_pServer (NULL)
		, m_pClient(client)
		, m_bRun(false)
		, m_eType (type)
		, m_iListenId(-1)
	{
		m_iHandle = epoll_create(MAX_EVENT_SIZE);
		if (m_iHandle == -1)
		{
			printf("epoll_create() Error.");
		}
	}

	CReactor_Epoll::~CReactor_Epoll()
	{
		if (m_iHandle != -1)
		{
			::close(m_iHandle);
		}
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

		if (m_eType == REACTOR_LISTEN)
			m_iListenId = sockId;

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
			printf("epoll_ctl remove error %d", errno);
			return false;
		}

		return true;
	}

	void CReactor_Epoll::Loop(int32 timeout)
	{
		if (m_eType == REACTOR_READ)
		{
			std::thread thread2(std::bind(&CReactor_Epoll::HandleMessage, this));
			LoopThread(timeout);

			thread2.join();
		}
		else
			LoopThread(timeout);
	}

	void CReactor_Epoll::HandleMessage()
	{
		socket_t sockId;
		char *message = new char[MAX_MESSAGE_LEGNTH];

		const char* buf;
		uint32_t len;

		do
		{
			sockId = -1;

			{
				std::unique_lock<std::recursive_mutex> lock(m_mutexRecvBuf);
				if (!m_setRecvSock.empty())
				{
					sockId = *m_setRecvSock.begin();
					m_setRecvSock.erase(m_setRecvSock.begin());

					buf = m_mapRecvBuf[sockId].PopBuf(len);
					memcpy(message, buf, len);
				}
				else
				{
					m_conditionRead.wait_for(lock, std::chrono::seconds(1));
					continue;
				}
			}

			if (sockId != -1)
			{
				if (m_pServer != NULL)
					m_pServer->OnMessage(sockId, message, len);
				else if (m_pClient != NULL)
					m_pClient->OnMessage(sockId, message, len);
			}
		}while (m_bRun);

		delete []message;
	}

	void CReactor_Epoll::LoopThread(int32 timeout)
	{
		char* buf1 = new char[MAX_MESSAGE_LEGNTH];

		m_bRun = true;
		int32_t result;

		//1. 接收连接
		if (m_eType == REACTOR_LISTEN)
		{
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
						newSock = DoAccept();
						if (newSock != -1)
						{
							//通知tcpserver
							m_pServer->OnAccept(newSock);
						}

						CtlEvent(m_iListenId, EVENT_READ);
					}
				}
			}
		}
		//2. 读
		else if (m_eType == REACTOR_READ)
		{
			int readLen;

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
						break;
					}
				}

				if (result == 0)
				{
					continue;
				}

				for (int32_t i = 0; i<result; i++)
				{
					if (m_aEvents[i].events & EPOLLIN)
					{
						readLen = Recv(m_aEvents[i].data.fd, buf1);
						if (readLen > 0)
						{
							CtlEvent(m_aEvents[i].data.fd, EVENT_READ);

							{
								std::unique_lock<std::recursive_mutex> lock(m_mutexRecvBuf);
								m_mapRecvBuf[m_aEvents[i].data.fd].AddBuf(buf1, readLen);
								m_setRecvSock.insert(m_aEvents[i].data.fd);
								m_conditionRead.notify_one();
							}
							
							/*if (m_pServer != NULL)
								m_pServer->OnMessage(m_aEvents[i].data.fd, buf1, readLen);
							else if (m_pClient != NULL)
								m_pClient->OnMessage(m_aEvents[i].data.fd, buf1, readLen);*/
						}
						else if (readLen == -1)
						{
							if (m_pServer != NULL)
								m_pServer->OnClose(m_aEvents[i].data.fd);
							else if (m_pClient != NULL)
								m_pClient->OnClose(m_aEvents[i].data.fd);
						}
					}				
				}
			}
		}
		//3. 写
		else if (m_eType == REACTOR_WRITE)
		{
			int sendLen;
			socket_t sockId;
			const char *msg;
			uint32_t len;

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
							std::unique_lock<std::recursive_mutex> lock(m_mutexSendBuf);
							msg = m_mapSendBuf[sockId].PopBuf(len);
							memcpy (buf1, msg, len);
						}

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
		}

		delete []buf1;
	}

	uint32_t CReactor_Epoll::ConvertEventMask(uint32_t e)
	{
		uint32_t op = 0;
		if (e & EVENT_READ) op |= (EPOLLIN | EPOLLONESHOT);
		if (e & EVENT_WRITE) op |= (EPOLLOUT | EPOLLONESHOT);
		if (e & EVENT_ERROR) op |= EPOLLERR;
		return op;
	}

	void CReactor_Epoll::ReadySendMessage(socket_t sockId, const char* message, uint32_t len)
	{
		bool res = false;
		int retryNum = 3;

		do
		{
			{
				std::unique_lock<std::recursive_mutex> lock(m_mutexSendBuf);
				res = m_mapSendBuf[sockId].AddBuf(message, len);
			}
			if (res == true)
				break;

			std::this_thread::sleep_for(std::chrono::milliseconds(3));
		}while (--retryNum >= 0);

		if (res == false)
			printf("ReadySendMessage failed \n");

		CtlEvent (sockId, EVENT_WRITE);
	}

	void CReactor_Epoll::OnClose(socket_t sockId)
	{
		if (m_eType == REACTOR_WRITE)
		{
			std::unique_lock<std::recursive_mutex> lock(m_mutexSendBuf);
			m_mapSendBuf.erase(sockId);
		}
		else if (m_eType == REACTOR_READ)
		{
			std::unique_lock<std::recursive_mutex> lock(m_mutexRecvBuf);
			m_mapRecvBuf.erase(sockId);
			m_setRecvSock.erase(sockId);
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

		int sendMax = MAX_MESSAGE_LEGNTH;
		if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&sendMax, sizeof(sendMax)) == -1)
		{
			return false;
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

		if (::listen(listen_socket, 8) == -1)
		{
			printf("socket listen faild, errno %d.", errno);
			return false;
		}

		m_iListenId = listen_socket;
		AddEvent(m_iListenId, EVENT_READ);

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
				break;
		}while(--retryNum >= 0);
		
		return sockId;
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
