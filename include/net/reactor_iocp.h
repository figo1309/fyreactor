/************************************************************************/
/*
create time:	2015/6/3
athor:			葛飞跃
discribe:		iocp响应器头文件
*/
/************************************************************************/

#ifndef __REACTOR_IOCP_H__
#define __REACTOR_IOCP_H__

#include <set>
#include <condition_variable>
#include "../util/buffer.h"
#include "reactor.h"

#ifdef HAVE_IOCP

namespace fyreactor
{
	class CTCPServerImpl;
	class CTCPClientImpl;

	class CReactor_Iocp:public CReactor
	{
	public:
		CReactor_Iocp(CTCPServerImpl* server);
		CReactor_Iocp(CTCPClientImpl* client);
		virtual ~CReactor_Iocp();

		bool Listen(const std::string& ip, int port);
		socket_t Connect(const std::string& ip, int port, int myPort=0);
		void Loop(int32 timeout);
		bool AddEvent(socket_t sockId, uint32_t e);
		bool CtlEvent(socket_t sockId, uint32_t e);
		bool DelEvent(socket_t sockId);
		void Stop(){m_bRun = false;}
		void ReadySendMessage(socket_t sockId, const char* message, uint32_t len);
		void OnClose(socket_t sockId);

	protected:	
		void DoAccept();
		bool Recv(socket_t sockId);
		bool Send(socket_t sockId, const char* message, uint32_t len);

	private:
		void LoopThread(int32 timeout);
		socket_t CreateNewSocket();
		bool	InitSocket(socket_t sockId);

	private:
		CTCPServerImpl*						m_pServer;
		CTCPClientImpl*						m_pClient;
		handle_t							m_iHandle;
		bool								m_bRun;
		socket_t							m_iListenId;
		

		IO_DATA									m_acceptIoData;
		socket_t								m_iNewSockId;

		std::unordered_map<socket_t, IO_DATA>	m_mapRecvIoData;
		std::unordered_map<socket_t, IO_DATA>	m_mapSendIoData;
	};


}

#endif //HAVE_IOCP

#endif //__REACTOR_IOCP_H__
