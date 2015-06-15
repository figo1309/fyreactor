/************************************************************************/
/*
create time:	2015/6/10
athor:			∏∑…‘æ
discribe:		–‘ƒ‹≤‚ ‘
*/
/************************************************************************/

#ifndef __PROFILE_TEST_H__
#define __PROFILE_TEST_H__

#include "../define.hpp"

namespace fyreactor
{
	class CProfileTest
	{
	public:
		CProfileTest(const std::string& msg);
		~CProfileTest();

	private:
		uint32_t	m_iBeginTime;
		std::string	m_strMsg;
	};

}

#endif
