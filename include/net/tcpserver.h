/************************************************************************/
/*
create time:	2015/6/3
athor:			葛飞跃
discribe:		tcp服务器头文件
*/
/************************************************************************/

#ifndef __TCPSERVER_H__
#define __TCPSERVER_H__

#include "define.hpp"
#include "../util/timer.h"

namespace fyreactor
{
	class CTCPServer;
	typedef std::function<void(CTCPServer*, int, socket_t)> AcceptFunc;
	typedef std::function<void(socket_t, const char* message, uint32_t len)> MessageFunc;
	typedef std::function<void(socket_t)> CloseFunc;

	class CTCPServerImpl;

	class CTCPServer : public nocopyable
	{
	public:
		CTCPServer(int token, std::recursive_mutex* mutex = NULL);
		virtual ~CTCPServer();

		bool Listen(const std::string& ip, int port);
		void Run();		
		void ReadySendMessage(socket_t sockId, const char* message, uint32_t len);
		void Stop();
		void Close(socket_t sockId);

		void RegAcceptFunc(AcceptFunc func);
		void RegMessageFunc(MessageFunc func);
		void RegCloseFunc(CloseFunc func);

	private:
		CTCPServerImpl*				m_pTCPServerImpl;
	};
}

#endif //__TCPSERVER_H__
