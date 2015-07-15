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
#include "../util/timer.h"

typedef std::function<void(socket_t, const char* message, uint32_t len)> MessageFunc;
typedef std::function<void(socket_t)> CloseFunc;

namespace fyreactor
{
	class CTCPClientImpl;

	class CTCPClient : public nocopyable
	{
	public:
		CTCPClient(std::recursive_mutex* mutex = NULL);
		virtual ~CTCPClient();

		socket_t Connect(const std::string& ip, int port);
		void Run();
		void ReadySendMessage(socket_t sockId, const char* message, uint32_t len);
		void Stop();
		void Close(socket_t sockId);
		CTimerThread& GetTimerThread();

		void RegMessageFunc(MessageFunc func);
		void RegCloseFunc(CloseFunc func);

	private:
		CTCPClientImpl*				m_pTCPClientImpl;

	};
}

#endif //__TCPCLIENT_H__
