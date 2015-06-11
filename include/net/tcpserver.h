/************************************************************************/
/*
create time:	2015/6/3
athor:			���Ծ
discribe:		tcp������ͷ�ļ�
*/
/************************************************************************/

#ifndef __TCPSERVER_H__
#define __TCPSERVER_H__

#include <set>
#include "define.hpp"
#include "reactorgroup.h"
#include "../util/timer.h"

typedef std::function<void(socket_t)> AcceptFunc;
typedef std::function<void(socket_t, const char* message, uint32_t len)> MessageFunc;
typedef std::function<void(socket_t)> CloseFunc;

namespace fyreactor
{
	class CTCPServer
	{
	public:
		//���º���Ϊ����ʹ���ߵĵ��ýӿ�
		CTCPServer(std::recursive_mutex* mutex = NULL);
		~CTCPServer();

		bool Listen(const std::string& ip, int port);
		void Run();		
		void ReadySendMessage(socket_t sockId, const char* message, uint32_t len);
		void Stop();
		void Close(socket_t sockId);
		CTimerThread& GetTimerThread(){ return m_timerThread; }

		void RegAcceptFunc(AcceptFunc func){ m_acceptFunc = func; }
		void RegMessageFunc(MessageFunc func){ m_messageFunc = func; }
		void RegCloseFunc(CloseFunc func){ m_closeFunc = func; }

	public:
		//���º����������ڲ��Ľӿ�
		void OnAccept(socket_t sockId);
		void OnMessage(socket_t sockId, const char* message, uint32_t len);
		void OnClose(socket_t sockId);

	private:
		
		std::recursive_mutex*			m_outMutex;
		CReactorGroup					m_reactorGroup;
		std::recursive_mutex			m_socketMutex;
		std::set<socket_t>				m_setSocket;
		CTimerThread					m_timerThread;

		AcceptFunc						m_acceptFunc;
		MessageFunc						m_messageFunc;
		CloseFunc						m_closeFunc;

	};
}

#endif //__TCPSERVER_H__
