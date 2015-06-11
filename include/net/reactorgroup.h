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

#define SERVER_REACTOR_NUM 3		//必须大于等于3
#define CLIENT_REACTOR_NUM 2		//暂时必须为2

namespace fyreactor
{
	class CTCPServer;
	class CTCPClient;

	class CReactorGroup
	{
	public:
		CReactorGroup(CTCPServer *server);
		CReactorGroup(CTCPClient *client);
		~CReactorGroup();

		void StartListen(const std::string& ip, int port);
		socket_t Connect(const std::string& ip, int port);
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
	};	
}

#endif //__REACTOR_GROUP_H__
