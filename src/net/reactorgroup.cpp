/************************************************************************/
/*
create time:	2015/6/3
athor:			葛飞跃
discribe:		响应器群实现
*/
/************************************************************************/
#include <net/tcpserver_impl.h>
#include <net/reactorgroup.h>
#include <net/reactor_epoll.h>
#include <net/reactor_iocp.h>

namespace fyreactor
{

	CReactorGroup::CReactorGroup(CTCPServerImpl *server)
		: m_bServerOrClient(true)
		, m_iReactorNum(SERVER_REACTOR_NUM)
	{
		m_aTreadReactor = new SReactorTread[m_iReactorNum];
		
#ifdef HAVE_EPOLL
		//1.最后一个reactor是接收连接reactor
		//2.偶数reactor是读reactor
		//3.奇数reactor是写reactor
		handle_t listenHandle = epoll_create(MAX_EVENT_SIZE);
		handle_t readHandle = epoll_create(MAX_EVENT_SIZE);
		handle_t writeHandle = epoll_create(MAX_EVENT_SIZE);
		if ((listenHandle == -1) || (readHandle == -1) || (writeHandle == -1))
		{
			printf("epoll_create() Error.");
		}

		m_aTreadReactor[m_iReactorNum - 1].m_pReactor = new CReactor_Epoll(server, REACTOR_LISTEN, listenHandle, m_mutexSendBuf, m_mapSendBuf, m_mutexReadEpoll);
		
		for (int i = 0; i < m_iReactorNum - 1; i += 2)
		{
			m_aTreadReactor[i].m_pReactor = new CReactor_Epoll(server, REACTOR_READ, readHandle, m_mutexSendBuf, m_mapSendBuf, m_mutexReadEpoll);			
		}
		for (int i = 1; i < m_iReactorNum - 1; i += 2)
		{
			m_aTreadReactor[i].m_pReactor = new CReactor_Epoll(server, REACTOR_WRITE, writeHandle, m_mutexSendBuf, m_mapSendBuf, m_mutexWriteEpoll);
		}

		
#elif defined HAVE_IOCP
		m_aTreadReactor[0].m_pReactor = new CReactor_Iocp(server);
#endif
	}

	CReactorGroup::CReactorGroup(CTCPClientImpl *client)
		: m_bServerOrClient(false)
		, m_iReactorNum(CLIENT_REACTOR_NUM)
	{
		m_aTreadReactor = new SReactorTread[m_iReactorNum];
#ifdef HAVE_EPOLL
		//2.偶数reactor是读reactor
		//3.奇数reactor是写reactor
		handle_t readHandle = epoll_create(MAX_EVENT_SIZE);
		handle_t writeHandle = epoll_create(MAX_EVENT_SIZE);
		if ((readHandle == -1) || (writeHandle == -1))
		{
			printf("epoll_create() Error.");
		}

		for (int i = 0; i < m_iReactorNum; i += 2)
		{
			m_aTreadReactor[i].m_pReactor = new CReactor_Epoll(client, REACTOR_READ, readHandle, m_mutexSendBuf, m_mapSendBuf, m_mutexReadEpoll);
		}
		for (int i = 1; i < m_iReactorNum; i += 2)
		{
			m_aTreadReactor[i].m_pReactor = new CReactor_Epoll(client, REACTOR_WRITE, writeHandle, m_mutexSendBuf, m_mapSendBuf, m_mutexWriteEpoll);
		}
#elif defined HAVE_IOCP
		m_aTreadReactor[0].m_pReactor = new CReactor_Iocp(client);
#endif
	}

	CReactorGroup::~CReactorGroup()
	{
		for (int i = 0; i < m_iReactorNum; ++i)
		{
			if (m_aTreadReactor[i].m_pReactor != NULL)
			{
				m_aTreadReactor[i].m_pReactor->Stop();

				delete m_aTreadReactor[i].m_pReactor;
				m_aTreadReactor[i].m_pReactor = NULL;
			}
		}

		delete[]m_aTreadReactor;
	}

	void CReactorGroup::StartListen(const std::string& ip, int port)
	{
		if (m_bServerOrClient == true)
		{
			//最后一个reactor是专门的接收连接reactor
			m_aTreadReactor[m_iReactorNum - 1].m_pReactor->Listen(ip, port);
		}
	}

	socket_t CReactorGroup::Connect(const std::string& ip, int port)
	{
		if (m_bServerOrClient == false)
		{
			socket_t sockId = m_aTreadReactor[0].m_pReactor->Connect(ip, port);

			if (sockId != -1)
			{
#ifdef HAVE_EPOLL
				if (!m_aTreadReactor[0].m_pReactor->AddEvent(sockId, EVENT_READ))
					return -1;

				if (!m_aTreadReactor[1].m_pReactor->AddEvent(sockId, 0))
					return -1;
#elif defined HAVE_IOCP
				if (!m_aTreadReactor[0].m_pReactor->AddEvent(sockId, EVENT_READ))
					return -1;
#endif
			}

			return sockId;
		}

		return -1;
	}
	
	void CReactorGroup::Run()
	{
		for (int i = 0; i < m_iReactorNum - 1; ++i)
		{
			m_aTreadReactor[i].m_pThread = new std::thread(std::bind(&CReactor::Loop, m_aTreadReactor[i].m_pReactor, 1000));
		}
		m_aTreadReactor[m_iReactorNum - 1].m_pReactor->Loop(1000);

		for (int i = 0; i < m_iReactorNum - 1; ++i)
		{
			m_aTreadReactor[i].m_pThread->join();
			delete m_aTreadReactor[i].m_pThread;
			m_aTreadReactor[i].m_pThread = NULL;
		}
	}

	void CReactorGroup::OnNewConnection(socket_t sockId)
	{
		if (m_bServerOrClient)
		{
#ifdef HAVE_EPOLL
			m_aTreadReactor[0].m_pReactor->AddEvent(sockId, EVENT_READ);
			m_aTreadReactor[1].m_pReactor->AddEvent(sockId, 0);
#elif defined HAVE_IOCP
			m_aTreadReactor[0].m_pReactor->AddEvent(sockId, EVENT_READ);
#endif
		}
	}

	void CReactorGroup::ReadySendMessage(socket_t sockId, const char* message, uint32_t len)
	{
#ifdef HAVE_EPOLL
		m_aTreadReactor[1].m_pReactor->ReadySendMessage(sockId, message, len);
#elif defined HAVE_IOCP
		m_aTreadReactor[0].m_pReactor->ReadySendMessage(sockId, message, len);
#endif
	}

	void CReactorGroup::Stop()
	{
		for (int i = 0; i < m_iReactorNum; ++i)
		{
			m_aTreadReactor[i].m_pReactor->Stop();
		}
	}

	void CReactorGroup::OnClose(socket_t sockId)
	{
		for (int i = 0; i < m_iReactorNum; ++i)
		{
			m_aTreadReactor[i].m_pReactor->OnClose(sockId);
		}
	}

}
