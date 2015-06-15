/************************************************************************/
/*
create time:	2015/6/10
athor:			∏∑…‘æ
discribe:		–‘ƒ‹≤‚ ‘
*/
/************************************************************************/

#include <util/timer.h>
#include <util/profile_test.h>

namespace fyreactor
{

	CProfileTest::CProfileTest(const std::string& msg)
		: m_iBeginTime(CTimerThread::GetMilSec())
		, m_strMsg(msg)
	{

	}

	CProfileTest::~CProfileTest()
	{
		uint32_t lastTime = CTimerThread::GetMilSec() - m_iBeginTime;
		if (lastTime > 5)
			printf("1111 %s  %d \n", m_strMsg.c_str(), lastTime);
	}


}

