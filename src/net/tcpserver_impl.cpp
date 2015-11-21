/************************************************************************/
/*
create time:	2015/6/3
athor:			���Ծ
discribe:		tcp������ʵ��
*/
/************************************************************************/
#include <net/tcpserver_impl.h>

namespace fyreactor
{
	CTCPServerImpl::CTCPServerImpl(CTCPServer* server, int token, std::recursive_mutex* mutex)
		: m_server(server)
		, m_token(token)
		, m_outMutex(mutex)
		, m_reactorGroup(this)
		, m_acceptFunc(nullptr)
		, m_messageFunc(nullptr)
		, m_closeFunc(nullptr)		
	{
	}

	CTCPServerImpl::~CTCPServerImpl()
	{
	}

	bool CTCPServerImpl::Listen(const std::string& ip, int port)
	{
		m_reactorGroup.StartListen(ip, port);
		return true;
	}

	void CTCPServerImpl::Run()
	{
		m_reactorGroup.Run();
	}

	void CTCPServerImpl::OnAccept(socket_t sockId)
	{
		//1.�µ�socket���뵽reactor������
		m_reactorGroup.OnNewConnection(sockId);

		//2.�����µ����Ӷ���
		{
			std::lock_guard<std::recursive_mutex> lock(m_socketMutex);
			m_setSocket.insert(sockId);
		}
		//3.ִ�лص�����
		{
			if (m_acceptFunc != nullptr)
			{
				if (m_outMutex != NULL)
				{
					std::lock_guard<std::recursive_mutex> lock(*m_outMutex);
					m_acceptFunc(m_server, m_token, sockId);
				}
				else
				{
					m_acceptFunc(m_server, m_token, sockId);
				}

			}
		}		
	}

	void CTCPServerImpl::OnMessage(socket_t sockId, const char* message, uint32_t len)
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

	void CTCPServerImpl::ReadySendMessage(socket_t sockId, const char* message, uint32_t len)
	{
		m_reactorGroup.ReadySendMessage(sockId, message, len);
	}

	void CTCPServerImpl::Stop()
	{
		m_reactorGroup.Stop();
	}

	void CTCPServerImpl::Close(socket_t sockId)
	{
#ifdef HAVE_EPOLL
		::close(sockId);
#elif defined HAVE_IOCP
		::closesocket(sockId);
#endif
	}

	void CTCPServerImpl::OnClose(socket_t sockId)
	{
		{
			std::lock_guard<std::recursive_mutex> lock(m_socketMutex);
			auto findIter = m_setSocket.find(sockId);
			if (findIter != m_setSocket.end())
				m_setSocket.erase(sockId);
			else
				return;
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

