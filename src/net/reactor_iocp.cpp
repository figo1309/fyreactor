/************************************************************************/
/*
create time:	2015/6/3
athor:			葛飞跃
discribe:		iocp响应器实现
*/
/************************************************************************/
#include <util/profile_test.h>
#include <net/define.hpp>
#include <net/tcpserver.h>
#include <net/tcpclient.h>
#include <net/reactor_iocp.h>

#ifdef HAVE_IOCP

namespace fyreactor
{
	CReactor_Iocp::CReactor_Iocp(CTCPServer* server)
		: m_pServer (server)
		, m_pClient(NULL)
		, m_bRun(false)
		, m_iListenId(0)
		, m_iNewSockId(0)
	{

		WSADATA wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		{
			printf("WSAStartup ERROR.");
		}

		m_iHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
		if (!m_iHandle)
		{
			printf("CreateIoCompletionPort() Error.");
		}

		memset(&m_acceptIoData, 0, sizeof(IO_DATA));
		m_acceptIoData.socket = 0;
		m_acceptIoData.buf.buf = new char[MAX_MESSAGE_LEGNTH];
		m_acceptIoData.buf.len = MAX_MESSAGE_LEGNTH;
	}

	CReactor_Iocp::CReactor_Iocp(CTCPClient* client)
		: m_pServer (NULL)
		, m_pClient(client)
		, m_bRun(false)
		, m_iListenId(0)
		, m_iNewSockId(0)
	{
		WSADATA wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		{
			printf("WSAStartup ERROR.");
		}

		m_iHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
		if (!m_iHandle)
		{
			printf("CreateIoCompletionPort() Error.");
		}

		memset(&m_acceptIoData, 0, sizeof(IO_DATA));
		m_acceptIoData.socket = 0;
		m_acceptIoData.buf.buf = new char[MAX_MESSAGE_LEGNTH];
		m_acceptIoData.buf.len = MAX_MESSAGE_LEGNTH;
	}

	CReactor_Iocp::~CReactor_Iocp()
	{
		CloseHandle(m_iHandle);

		WSACleanup();

		if (m_acceptIoData.buf.buf != NULL)
		{
			delete[]m_acceptIoData.buf.buf;
		}
		for (auto iter = m_mapRecvIoData.begin(); iter != m_mapRecvIoData.end(); ++iter)
		{
			delete[](iter->second.buf.buf);
		}
		for (auto iter = m_mapSendIoData.begin(); iter != m_mapSendIoData.end(); ++iter)
		{
			delete[](iter->second.buf.buf);
		}
	}	

	bool CReactor_Iocp::AddEvent(socket_t sockId, uint32_t e)
	{
		if (!CreateIoCompletionPort((HANDLE)sockId, m_iHandle, 0, 0))
		{
			printf("CreateIoCompletionPort Error %d", WSAGetLastError());
			return false;
		}

		if (e == (uint32_t)EVENT_READ)
		{
			if (m_mapRecvIoData.find(sockId) == m_mapRecvIoData.end())
			{
				IO_DATA& recvIoData = m_mapRecvIoData[sockId];
				memset(&recvIoData, 0, sizeof(IO_DATA));
				recvIoData.socket = sockId;
				recvIoData.buf.buf = new char[MAX_MESSAGE_LEGNTH];
				recvIoData.buf.len = MAX_MESSAGE_LEGNTH;
			}

			if (m_mapSendIoData.find(sockId) == m_mapSendIoData.end())
			{
				IO_DATA& sendIoData = m_mapSendIoData[sockId];
				memset(&sendIoData, 0, sizeof(IO_DATA));
				sendIoData.socket = sockId;
				sendIoData.buf.buf = new char[MAX_MESSAGE_LEGNTH];
				sendIoData.buf.len = MAX_MESSAGE_LEGNTH;
			}

			if (!Recv(sockId))
				return false;
		}
		else if (e == (uint32_t)EVENT_OPEN)
		{
			DoAccept();
		}

		return true;
	}

	bool CReactor_Iocp::CtlEvent(socket_t sockId, uint32_t e)
	{
		if (e == EVENT_READ)
		{
			if (!Recv(sockId))
				return false;
		}
		else if (e == EVENT_OPEN)
		{
			DoAccept();
		}

		return true;
	}

	bool CReactor_Iocp::DelEvent(socket_t sockId)
	{
		auto iter = m_mapRecvIoData.find(sockId);
		if (iter != m_mapRecvIoData.end())
		{
			delete[](iter->second.buf.buf);
			m_mapRecvIoData.erase(iter);
		}

		iter = m_mapSendIoData.find(sockId);
		if (iter != m_mapSendIoData.end())
		{
			delete[](iter->second.buf.buf);
			m_mapSendIoData.erase(iter);
		}

		return true;
	}

	void CReactor_Iocp::Loop(int32 timeout)
	{
		LoopThread(timeout);
	}

	void CReactor_Iocp::LoopThread(int32 timeout)
	{
		m_bRun = true;

		DWORD count_bytes = 0;
		ULONG_PTR completion_key = NULL;
		LPOVERLAPPED overlapped = NULL;

		{
			while (m_bRun)
			{
				bool ret = GetQueuedCompletionStatus(m_iHandle, &count_bytes, &completion_key, &overlapped, timeout < 0 ? INFINITE : timeout);

				if (!ret && !overlapped)
				{
					//printf("GetQueuedCompletionStatus error %d", WSAGetLastError());
					continue;
				}

				IO_DATA* io_data = (IO_DATA*)overlapped;
				io_data->bytes = count_bytes;

				if (io_data->type == HANDLE_COMPLETE)
				{
					continue;
				}

				switch (io_data->type)
				{
				case IO_TYPE::HANDLE_ACCEPT:
				{					
					if (m_iNewSockId != 0)
					{
						//通知tcpserver
						m_pServer->OnAccept(m_iNewSockId);
					}
					CtlEvent(m_iListenId, EVENT_OPEN);
					break;
				}
				case IO_TYPE::HANDLE_RECV:
				{
					if (io_data->bytes > 0)
					{
						if (m_pServer != NULL)
							m_pServer->OnMessage(io_data->socket, io_data->buf.buf, io_data->bytes);
						else if (m_pClient != NULL)
							m_pClient->OnMessage(io_data->socket, io_data->buf.buf, io_data->bytes);

						CtlEvent(io_data->socket, EVENT_READ);
					}
					else
					{
						if (m_pServer != NULL)
							m_pServer->OnClose(io_data->socket);
						else if (m_pClient != NULL)
							m_pClient->OnClose(io_data->socket);
					}
					break;
				}
				case IO_TYPE::HANDLE_SEND:
				{
					break;
				}
				default:
				{
					QING_TRACE();
					break;
				}
				}
			}
		}
	}

	void CReactor_Iocp::ReadySendMessage(socket_t sockId, const char* message, uint32_t len)
	{
		Send(sockId, message, len);
	}

	void CReactor_Iocp::OnClose(socket_t sockId)
	{
		DelEvent(sockId);
	}

	socket_t CReactor_Iocp::CreateNewSocket()
	{
		socket_t sock = 0;
#ifdef HAVE_IOCP
		sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
#elif defined(HAVE_EPOLL)
		sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#elif defined(HAVE_KQUEUE)
		sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#endif

		return sock;
	}

	bool CReactor_Iocp::InitSocket(socket_t sock)
	{
		if (sock == 0)
		{
			return false;
		}

		int i = 1;
		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&i, sizeof(int)) == -1)
		{
			printf("setsocketopt SO_REUSEADDR error");
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

		return true;
	}

	void CReactor_Iocp::DoAccept()
	{
		socket_t sockId = CreateNewSocket();
		if (sockId == 0)
			return;

		if (InitSocket(sockId) == false)
			return;

		/* 设置异步 IO 数据内容 */
		//memset(&m_acceptIoData, 0, sizeof(IO_DATA));
		memset(m_acceptIoData.buf.buf, 0, MAX_MESSAGE_LEGNTH);

		m_acceptIoData.type = IO_TYPE::HANDLE_ACCEPT;
		m_acceptIoData.socket = m_iListenId;

		if (!AcceptEx(m_iListenId,
			sockId,
			m_acceptIoData.buf.buf,
			0,
			sizeof(struct sockaddr_in) + 16,
			sizeof(struct sockaddr_in) + 16,
			&m_acceptIoData.bytes,
			&m_acceptIoData))
		{
			uint32_t wsa_last_error = WSAGetLastError();
			if (wsa_last_error != ERROR_SUCCESS && wsa_last_error != ERROR_IO_PENDING)
			{
				printf("AcceptEx error\n");
				return;
			}
		}

		m_iNewSockId = sockId;
	}

	bool CReactor_Iocp::Listen(const std::string& ip, int port)
	{
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(ip.c_str());
		addr.sin_port = htons(port);

		socket_t listen_socket = CreateNewSocket();
		if (listen_socket == 0)
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
		if (!AddEvent(m_iListenId, EVENT_OPEN))
			return false;

		return true;
	}

	socket_t CReactor_Iocp::Connect(const std::string& ip, int port)
	{
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(ip.c_str());
		addr.sin_port = htons(port);

		socket_t sockId = CreateNewSocket();
		if (!InitSocket(sockId))
			return 0;

		int retryNum = 3;
		do
		{
			if (::connect(sockId, (sockaddr*)&addr, sizeof(sockaddr)) != SOCKET_ERROR)
				break;
		}while(--retryNum >= 0);
		
		//AddEvent(sockId, EVENT_READ);
		return sockId;
	}

	bool CReactor_Iocp::Recv(socket_t sockId)
	{
		auto sockIter = m_mapRecvIoData.find(sockId);
		if ((sockIter == m_mapRecvIoData.end()) || (sockIter->second.type == IO_TYPE::HANDLE_COMPLETE))
		{
			return false;
		}
		IO_DATA& recvIoData = sockIter->second;
		recvIoData.bytes = 0;
		recvIoData.type = HANDLE_RECV;

		DWORD flags = 0;
		if (WSARecv(sockId, &recvIoData.buf, 1, &recvIoData.bytes, &flags, &recvIoData, NULL) != 0)
		{
			uint32_t wsa_last_error = WSAGetLastError();
			if (wsa_last_error != ERROR_SUCCESS && wsa_last_error != ERROR_IO_PENDING)
			{
				printf("WSARecv error %d.", wsa_last_error);
				closesocket(sockId);
				return false;
			}
		}
		return true;
	}

	bool CReactor_Iocp::Send(socket_t sockId, const char* message, uint32_t len)
	{
		auto iter = m_mapSendIoData.find(sockId);
		if (iter == m_mapSendIoData.end())
			return false;

		IO_DATA& sendData = iter->second;

		sendData.buf.len = len;
		memcpy(sendData.buf.buf, message, len);
		sendData.type = HANDLE_SEND;
		sendData.bytes = 0;
		int send_res = 0;

		send_res = WSASend(sockId, &sendData.buf, 1, &sendData.bytes, 0, &sendData, NULL);
		if (send_res != 0)
		{
			int err = 0;
			uint32_t wsa_last_error = WSAGetLastError();
			if ((send_res == SOCKET_ERROR) &&
				(WSA_IO_PENDING != (err = WSAGetLastError()))) {
				printf("WSASend failed with error\n");
				closesocket(sockId);
				return false;
			}
		}

		return true;
	}

}

#endif //HAVE_IOCP
