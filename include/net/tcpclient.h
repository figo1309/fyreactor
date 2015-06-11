/************************************************************************/
/*
create time:	2015/6/8
athor:			葛飞跃
discribe:		tcp客户端头文件
*/
/************************************************************************/

#ifndef __TCPCLIENT_H__
#define __TCPCLIENT_H__

#include <set>
#include "define.hpp"
#include "reactorgroup.h"
#include "../util/timer.h"

typedef std::function<void(socket_t, const char* message, uint32_t len)> MessageFunc;
typedef std::function<void(socket_t)> CloseFunc;

namespace fyreactor
{
	class CTCPClient
	{
	public:
		//以下函数为面向使用者的调用接口
		CTCPClient(std::recursive_mutex* mutex = NULL);
		~CTCPClient();

		socket_t Connect(const std::string& ip, int port);
		void Run();
		void ReadySendMessage(socket_t sockId, const char* message, uint32_t len);
		void Stop();
		void Close(socket_t sockId);
		CTimerThread& GetTimerThread(){ return m_timerThread; }

		void RegMessageFunc(MessageFunc func){ m_messageFunc = func; }
		void RegCloseFunc(CloseFunc func){ m_closeFunc = func; }

	public:
		//以下函数是面向内部的接口
		void OnMessage(socket_t sockId, const char* message, uint32_t len);
		void OnClose(socket_t sockId);

	private:
		std::recursive_mutex*			m_outMutex;
		std::set<socket_t>				m_setSocket;
		CTimerThread					m_timerThread;

		MessageFunc						m_messageFunc;
		CloseFunc						m_closeFunc;

		CReactorGroup					m_reactorGroup;

	};
}

#endif //__TCPSERVER_H__
