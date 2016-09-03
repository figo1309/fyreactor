/************************************************************************/
/*
create time:	2015/6/8
athor:			葛飞跃
discribe:		tcp客户端头文件
*/
/************************************************************************/

#ifndef __TCPCLIENT_IMPL_H__
#define __TCPCLIENT_IMPL_H__

#include <set>
#include "define.hpp"
#include "tcpclient.h"
#include "reactorgroup.h"
#include "../util/timer.h"

namespace fyreactor
{
	class CTCPClientImpl : public nocopyable
	{
	public:
		//以下函数为面向使用者的调用接口
		CTCPClientImpl(std::recursive_mutex* mutex = NULL);
		virtual ~CTCPClientImpl();

		socket_t Connect(const std::string& ip, int port, int myPort = 0);
		void Run();
		void ReadySendMessage(socket_t sockId, const char* message, uint32_t len);
		void Stop();
		void Close(socket_t sockId);

		void RegMessageFunc(MessageFunc func){ m_messageFunc = func; }
		void RegCloseFunc(CloseFunc func){ m_closeFunc = func; }

	public:
		//以下函数是面向内部的接口
		void OnMessage(socket_t sockId, const char* message, uint32_t len);
		void OnClose(socket_t sockId);

	private:
		std::recursive_mutex*			m_outMutex;
		std::set<socket_t>				m_setSocket;

		MessageFunc						m_messageFunc;
		CloseFunc						m_closeFunc;

		CReactorGroup					m_reactorGroup;

	};
}

#endif //__TCPCLIENT_IMPL_H__
