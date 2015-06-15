/************************************************************************/
/*
create time:	2015/6/3
athor:			¸ð·ÉÔ¾
discribe:		ÏìÓ¦Æ÷½Ó¿Ú
*/
/************************************************************************/

#ifndef __REACTOR_H__
#define __REACTOR_H__

#include "define.hpp"

enum EReactorType
{
	REACTOR_LISTEN = 1,
	REACTOR_WRITE,
	REACTOR_READ,
};

namespace fyreactor
{

	class CReactor
	{
	public:
		CReactor(){}
		virtual ~CReactor(){}

		virtual bool Listen(const std::string& ip, int port) = 0;
		virtual socket_t Connect(const std::string& ip, int port) = 0;
		virtual void Loop(int32 timeout) = 0;
		virtual bool AddEvent(socket_t sockId, uint32_t e) = 0;
		virtual bool CtlEvent(socket_t sockId, uint32_t e) = 0;
		virtual bool DelEvent(socket_t sockId) = 0;
		virtual void Stop() = 0;
		virtual void ReadySendMessage(socket_t sockId, const char* message, uint32_t len) = 0;
		virtual void OnClose(socket_t sockId) = 0;
	};


}

#endif
