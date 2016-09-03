/************************************************************************/
/*
create time:	2015/6/3
athor:			葛飞跃
discribe:		响应器群头文件
*/
/************************************************************************/

#ifndef __REACTOR_GROUP_H__
#define __REACTOR_GROUP_H__

#include "define.hpp"
#include "reactor_epoll.h"

#ifdef HAVE_EPOLL
#define SERVER_REACTOR_NUM 7
#define CLIENT_REACTOR_NUM 6
#elif defined HAVE_IOCP
#define SERVER_REACTOR_NUM 1		
#define CLIENT_REACTOR_NUM 1		
#endif

namespace fyreactor
{
	class CTCPServerImpl;
	class CTCPClientImpl;

	class CReactorGroup : public nocopyable
	{
	public:
		CReactorGroup(CTCPServerImpl *server);
		CReactorGroup(CTCPClientImpl *client);
		virtual ~CReactorGroup();

		void StartListen(const std::string& ip, int port);
		socket_t Connect(const std::string& ip, int port, int myPort = 0);
		void Run();
		void OnNewConnection(socket_t sockId);
		void ReadySendMessage(socket_t sockId, const char* message, uint32_t len);
		void Stop();
		void OnClose(socket_t sockId);

	private:
		struct SReactorTread
		{
			SReactorTread()
				: m_pThread(NULL)
				, m_pReactor(NULL)
			{
			}

			std::thread*	m_pThread;
			CReactor*		m_pReactor;
		};
		SReactorTread*		m_aTreadReactor;
		bool				m_bServerOrClient;
		int					m_iReactorNum;		

		std::mutex								m_mutexSendBuf;
		std::unordered_map<socket_t, CBuffer>	m_mapSendBuf;

		std::mutex								m_mutexReadEpoll;
		std::mutex								m_mutexWriteEpoll;
	};	
}

#endif //__REACTOR_GROUP_H__
