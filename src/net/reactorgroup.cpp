/************************************************************************/
/*
create time:	2015/6/3
athor:			���Ծ
discribe:		��Ӧ��Ⱥʵ��
*/
/************************************************************************/
#include <net/tcpserver.h>
#include <net/reactorgroup.h>
#include <net/reactor_epoll.h>
#include <net/reactor_iocp.h>

namespace fyreactor
{

	CReactorGroup::CReactorGroup(CTCPServer *server)
		: m_bServerOrClient(true)
		, m_iReactorNum(SERVER_REACTOR_NUM)
	{
		m_aTreadReactor = new SReactorTread[m_iReactorNum];
		
#ifdef HAVE_EPOLL
		//1.��һ��reactor�ǽ�������reactor
		//2.�ڶ���reactor��дreactor
		//3.����reactor�Ƕ�reactor
		m_aTreadReactor[0].m_pReactor = new CReactor_Epoll(server, REACTOR_LISTEN);
		m_aTreadReactor[1].m_pReactor = new CReactor_Epoll(server, REACTOR_WRITE);
		
		for (int i = 2; i < m_iReactorNum; ++i)
		{
			m_aTreadReactor[i].m_pReactor = new CReactor_Epoll(server, REACTOR_READ);
		}
#elif defined HAVE_IOCP
		m_aTreadReactor[0].m_pReactor = new CReactor_Iocp(server);
#endif
	}

	CReactorGroup::CReactorGroup(CTCPClient *client)
		: m_bServerOrClient(false)
		, m_iReactorNum(CLIENT_REACTOR_NUM)
	{
		m_aTreadReactor = new SReactorTread[m_iReactorNum];
#ifdef HAVE_EPOLL
		//1.��һ��reactor�Ƕ�reactor
		//2.�ڶ���reactor��дreactor

		m_aTreadReactor[0].m_pReactor = new CReactor_Epoll(client, REACTOR_READ);
		m_aTreadReactor[1].m_pReactor = new CReactor_Epoll(client, REACTOR_WRITE);
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
			//��һ��reactor��ר�ŵĽ�������reactor
			m_aTreadReactor[0].m_pReactor->Listen(ip, port);
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
				m_aTreadReactor[0].m_pReactor->AddEvent(sockId, EVENT_READ);
				m_aTreadReactor[1].m_pReactor->AddEvent(sockId, 0);
#elif defined HAVE_IOCP
				m_aTreadReactor[0].m_pReactor->AddEvent(sockId, EVENT_READ);
#endif
			}

			return sockId;
		}

		return -1;
	}
	
	void CReactorGroup::Run()
	{
		for (int i = 1; i < m_iReactorNum; ++i)
		{
			m_aTreadReactor[i].m_pThread = new std::thread(std::bind(&CReactor::Loop, m_aTreadReactor[i].m_pReactor, 10));
		}
		m_aTreadReactor[0].m_pReactor->Loop(10);

		for (int i = 1; i < m_iReactorNum; ++i)
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
			int chooseReactor = sockId % (m_iReactorNum - 2) + 2;
			m_aTreadReactor[chooseReactor].m_pReactor->AddEvent(sockId, EVENT_READ);

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