/************************************************************************/
/*
create time:	2015/6/8
athor:			���Ծ
discribe:		tcp�ͻ���ͷ�ļ�
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
		//���º���Ϊ����ʹ���ߵĵ��ýӿ�
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
		//���º����������ڲ��Ľӿ�
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
