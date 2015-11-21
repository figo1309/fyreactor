/************************************************************************/
/*
create time:	2015/6/3
athor:			葛飞跃
discribe:		tcp客户端实现
*/
/************************************************************************/
#include <net/tcpclient.h>
#include <net/tcpclient_impl.h>

namespace fyreactor
{
	CTCPClient::CTCPClient(std::recursive_mutex* mutex)
	{
		m_pTCPClientImpl = new CTCPClientImpl(mutex);
	}

	CTCPClient::~CTCPClient()
	{
		delete m_pTCPClientImpl;
	}

	socket_t CTCPClient::Connect(const std::string& ip, int port)
	{
		return m_pTCPClientImpl->Connect(ip, port);
	}

	void CTCPClient::Run()
	{
		m_pTCPClientImpl->Run();
	}

	void CTCPClient::ReadySendMessage(socket_t sockId, const char* message, uint32_t len)
	{
		m_pTCPClientImpl->ReadySendMessage(sockId, message, len);
	}

	void CTCPClient::Stop()
	{
		m_pTCPClientImpl->Stop();
	}

	void CTCPClient::Close(socket_t sockId)
	{
		m_pTCPClientImpl->Close(sockId);
	}

	void CTCPClient::RegMessageFunc(MessageFunc func)
	{
		m_pTCPClientImpl->RegMessageFunc(func);
	}

	void CTCPClient::RegCloseFunc(CloseFunc func)
	{
		m_pTCPClientImpl->RegCloseFunc(func);
	}
}

