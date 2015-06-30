/************************************************************************/
/*
create time:	2015/6/3
athor:			葛飞跃
discribe:		epoll响应器头文件
*/
/************************************************************************/

#ifndef __REACTOR_EPOLL_H__
#define __REACTOR_EPOLL_H__

#include <list>
#include <condition_variable>
#include "../util/buffer.h"
#include "reactor.h"

#ifdef HAVE_EPOLL

namespace fyreactor
{
	class CTCPServer;
	class CTCPClient;

	class CReactor_Epoll:public CReactor, public nocopyable
	{
	public:
		CReactor_Epoll(CTCPServer* server, EReactorType type, handle_t handle, \
			std::mutex& mutexSendBuf, \
			std::unordered_map<socket_t, CBuffer>&	mapSendBuf, \
			std::mutex&	mutexEpoll);

		CReactor_Epoll(CTCPClient* client, EReactorType type, handle_t handle, \
			std::mutex& mutexSendBuf, \
			std::unordered_map<socket_t, CBuffer>&	mapSendBuf, \
			std::mutex&	mutexEpoll);

		virtual ~CReactor_Epoll();

		bool Listen(const std::string& ip, int port);
		socket_t Connect(const std::string& ip, int port);
		void Loop(int32 timeout);
		bool AddEvent(socket_t sockId, uint32_t e);
		bool CtlEvent(socket_t sockId, uint32_t e);
		bool DelEvent(socket_t sockId);
		void Stop(){m_bRun = false;}
		void ReadySendMessage(socket_t sockId, const char* message, uint32_t len);
		void OnClose(socket_t sockId);

	private:	
		socket_t DoAccept();
		int Recv(socket_t sockId, char* buf);
		int Send(socket_t sockId, const char* message, uint32_t len);

		void LoopAccept(int32 timeout);
		void LoopRead(int32 timeout);
		void LoopWrite(int32 timeout);

		uint32_t ConvertEventMask(uint32_t e);
		socket_t CreateNewSocket();
		bool	InitSocket(socket_t sockId);

	private:
		CTCPServer*							m_pServer;
		CTCPClient*							m_pClient;
		handle_t							m_iHandle;
		struct epoll_event					m_aEvents[MAX_EVENT_SIZE];
		bool								m_bRun;
		EReactorType						m_eType;
		socket_t							m_iListenId;

		std::mutex&								m_mutexSendBuf;
		std::unordered_map<socket_t, CBuffer>&	m_mapSendBuf;

		std::mutex&							m_mutexEpoll;
	};


}

#endif //HAVE_EPOLL

#endif //__REACTOR_EPOLL_H__
