/************************************************************************/
/*
create time:	2015/6/3
athor:			葛飞跃
discribe:		tcp客户端实现
*/
/************************************************************************/
#include <net/tcpclient.h>

namespace fyreactor
{
	CTCPClient::CTCPClient(std::recursive_mutex* mutex)
		: m_outMutex(mutex)
		, m_timerThread(mutex)
		, m_messageFunc(nullptr)
		, m_closeFunc(nullptr)
		, m_reactorGroup(this)
	{

	}

	CTCPClient::~CTCPClient()
	{
	}

	socket_t CTCPClient::Connect(const std::string& ip, int port)
	{
		socket_t newSock = m_reactorGroup.Connect(ip, port);
		if (newSock != -1)
		{
			m_setSocket.insert(newSock);
		}

		return newSock;
	}

	void CTCPClient::Run()
	{
		m_timerThread.Run();

		m_reactorGroup.Run();
	}

	void CTCPClient::OnMessage(socket_t sockId, const char* message, uint32_t len)
	{
		if (m_messageFunc != nullptr)
		{
			if (m_outMutex != NULL)
			{
				std::lock_guard<std::recursive_mutex> lock(*m_outMutex);
				m_messageFunc(sockId, message, len);
			}
			else
			{
				m_messageFunc(sockId, message, len);
			}
		}
	}

	void CTCPClient::ReadySendMessage(socket_t sockId, const char* message, uint32_t len)
	{
		m_reactorGroup.ReadySendMessage(sockId, message, len);
	}

	void CTCPClient::Stop()
	{
		m_reactorGroup.Stop();

		m_timerThread.Stop();
	}

	void CTCPClient::Close(socket_t sockId)
	{
#ifdef HAVE_EPOLL
		::close(sockId);
#endif
	}

	void CTCPClient::OnClose(socket_t sockId)
	{
		m_setSocket.erase(sockId);
		if (m_closeFunc != nullptr)
		{			
			if (m_outMutex != NULL)
			{
				std::lock_guard<std::recursive_mutex> lock(*m_outMutex);
				m_closeFunc(sockId);
			}
			else
			{
				m_closeFunc(sockId);
			}
		}
	}
}

