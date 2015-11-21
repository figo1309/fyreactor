/************************************************************************/
/*
create time:	2015/6/3
athor:			���Ծ
discribe:		tcp������ͷ�ļ�
*/
/************************************************************************/

#ifndef __TCPSERVER_IMPL_H__
#define __TCPSERVER_IMPL_H__

#include <set>
#include "define.hpp"
#include "tcpserver.h"
#include "reactorgroup.h"
#include "../util/timer.h"

namespace fyreactor
{
	class CTCPServerImpl : public nocopyable
	{
	public:
		//���º���Ϊ����ʹ���ߵĵ��ýӿ�
		CTCPServerImpl(CTCPServer* server, int token, std::recursive_mutex* mutex = NULL);
		virtual ~CTCPServerImpl();

		bool Listen(const std::string& ip, int port);
		void Run();		
		void ReadySendMessage(socket_t sockId, const char* message, uint32_t len);
		void Stop();
		void Close(socket_t sockId);

		void RegAcceptFunc(AcceptFunc func){ m_acceptFunc = func; }
		void RegMessageFunc(MessageFunc func){ m_messageFunc = func; }
		void RegCloseFunc(CloseFunc func){ m_closeFunc = func; }

	public:
		//���º����������ڲ��Ľӿ�
		void OnAccept(socket_t sockId);
		void OnMessage(socket_t sockId, const char* message, uint32_t len);
		void OnClose(socket_t sockId);

	private:
		CTCPServer*						m_server;
		std::recursive_mutex*			m_outMutex;
		CReactorGroup					m_reactorGroup;
		std::recursive_mutex			m_socketMutex;
		std::set<socket_t>				m_setSocket;

		AcceptFunc						m_acceptFunc;
		MessageFunc						m_messageFunc;
		CloseFunc						m_closeFunc;

		int								m_token;

	};
}

#endif //__TCPSERVER_IMPL_H__
