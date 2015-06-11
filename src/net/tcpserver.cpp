/************************************************************************/
/*
create time:	2015/6/3
athor:			葛飞跃
discribe:		tcp服务器实现
*/
/************************************************************************/
#include <net/tcpserver.h>

namespace fyreactor
{
	CTCPServer::CTCPServer(std::recursive_mutex* mutex)
		: m_outMutex(mutex)
		, m_reactorGroup(this)
		, m_timerThread(mutex)
		, m_acceptFunc(nullptr)
		, m_messageFunc(nullptr)
		, m_closeFunc(nullptr)		
	{
	}

	CTCPServer::~CTCPServer()
	{
	}

	bool CTCPServer::Listen(const std::string& ip, int port)
	{
		m_reactorGroup.StartListen(ip, port);
		return true;
	}

	void CTCPServer::Run()
	{
		m_timerThread.Run();
		m_reactorGroup.Run();
	}

	void CTCPServer::OnAccept(socket_t sockId)
	{
		//1.新的socket加入到reactor管理中
		m_reactorGroup.OnNewConnection(sockId);

		//2.创建新的连接对象
		{
			std::lock_guard<std::recursive_mutex> lock(m_socketMutex);
			m_setSocket.insert(sockId);
		}
		//3.执行回调函数
		{
			if (m_acceptFunc != nullptr)
			{
				if (m_outMutex != NULL)
				{
					std::lock_guard<std::recursive_mutex> lock(*m_outMutex);
					m_acceptFunc(sockId);
				}
				else
				{
					m_acceptFunc(sockId);
				}

			}
		}		
	}

	void CTCPServer::OnMessage(socket_t sockId, const char* message, uint32_t len)
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

	void CTCPServer::ReadySendMessage(socket_t sockId, const char* message, uint32_t len)
	{
		m_reactorGroup.ReadySendMessage(sockId, message, len);
	}

	void CTCPServer::Stop()
	{
		m_reactorGroup.Stop();

		m_timerThread.Stop();
	}

	void CTCPServer::Close(socket_t sockId)
	{
#ifdef HAVE_EPOLL
		::close(sockId);
#endif
	}

	void CTCPServer::OnClose(socket_t sockId)
	{
		{
			std::lock_guard<std::recursive_mutex> lock(m_socketMutex);
			m_setSocket.erase(sockId);
		}

		m_reactorGroup.OnClose(sockId);

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

