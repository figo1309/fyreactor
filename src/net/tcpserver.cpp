/************************************************************************/
/*
create time:	2015/6/3
athor:			葛飞跃
discribe:		tcp服务器实现
*/
/************************************************************************/
#include <net/tcpserver.h>
#include <net/tcpserver_impl.h>

namespace fyreactor
{
	CTCPServer::CTCPServer(int token, std::recursive_mutex* mutex)
	{
		m_pTCPServerImpl = new CTCPServerImpl(this, token, mutex);
	}

	CTCPServer::~CTCPServer()
	{
		delete m_pTCPServerImpl;
	}

	bool CTCPServer::Listen(const std::string& ip, int port)
	{
		return m_pTCPServerImpl->Listen(ip, port);
	}

	void CTCPServer::Run()
	{
		m_pTCPServerImpl->Run();
	}


	void CTCPServer::ReadySendMessage(socket_t sockId, const char* message, uint32_t len)
	{
		m_pTCPServerImpl->ReadySendMessage(sockId, message, len);
	}

	void CTCPServer::Stop()
	{
		m_pTCPServerImpl->Stop();
	}

	void CTCPServer::Close(socket_t sockId)
	{
		m_pTCPServerImpl->Close(sockId);
	}

	void CTCPServer::RegAcceptFunc(AcceptFunc func)
	{
		m_pTCPServerImpl->RegAcceptFunc(func);
	}

	void CTCPServer::RegMessageFunc(MessageFunc func)
	{
		m_pTCPServerImpl->RegMessageFunc(func);
	}

	void CTCPServer::RegCloseFunc(CloseFunc func)
	{
		m_pTCPServerImpl->RegCloseFunc(func);
	}

}

