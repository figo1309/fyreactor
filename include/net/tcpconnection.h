/************************************************************************/
/*
create time:	2015/6/3
athor:			葛飞跃
discribe:		tcp连接接口
*/
/************************************************************************/

#ifndef __TCP_CONNECTION_H__
#define __TCP_CONNECTION_H__

#include "define.hpp"
#include "tcpserver.h"

namespace fyreactor
{

	class CTCPConnection
	{
	public:
		CTCPConnection(CTCPServer<CTCPConnection>* tcpServer, socket_t sockId)
			: m_iSockId(sockId)
		{
		}
		virtual ~CTCPConnection(){}

		void Send(char* message, uint32_t len)
		{
			m_pTcpServer->ReadySendMessage(m_iSockId, message, len);
		}
		virtual void OnMessage(char* message, uint32_t len) = 0;
		void Close(){ ::closesocket(m_iSockId); }
		virtual void OnClose() = 0;

	protected:

		socket_t							m_iSockId;
		CTCPServer<CTCPConnection>*			m_pTcpServer;
	};


}

#endif //__TCP_CONNECTION_H__
