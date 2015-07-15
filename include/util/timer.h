/************************************************************************/
/*
create time:	2015/6/5
athor:			葛飞跃
discribe:		定时器头文件
*/
/************************************************************************/
#ifndef __TIMER_H__
#define __TIMER_H__

#include <map>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "../define.hpp"

namespace fyreactor
{
	//单个定时器定义
	struct STimer
	{
		uint64 m_iId;
		uint64 m_iInterval;
		uint64 m_iNextTime;
		uint64 m_iStartTime;
		bool m_bOnce;
		bool m_bDeleted = false;
		std::function<void()> m_pFunc;


		bool operator < (STimer timer)
		{
			return m_iNextTime < timer.m_iNextTime;
		}
	};

	//定时器管理
	class CTimerThread : public nocopyable
	{
	public:
		CTimerThread(std::recursive_mutex* mutex = NULL);
		virtual ~CTimerThread();

		// interval_time: s
		uint64 Register(bool run_once, int interval_time, std::function<void()> fp);
		// interval_time: ms
		uint64 RegisterByMilSec(bool run_once, int interval_time, std::function<void()> fp);

		void UnRegister(uint64 timer_id);
		void Run();
		void Stop();

		static uint64_t GetMilSec();
		static uint64_t GetSec();

	private:
		void RunInThead();
		
		std::recursive_mutex*										m_outMutex;
		uint64_t													m_iCurretId;
		std::multimap<uint64, std::shared_ptr<STimer> >				m_mapOrderedTimers;
		std::unordered_map<uint64, std::shared_ptr<STimer> >		m_mapTimers;
		std::recursive_mutex										m_mutexTimer;
		std::condition_variable_any									m_condition;
		//std::recursive_mutex										m_mutexCondition;
		bool														m_bRun;
		std::thread*												m_pThread;
	};

}

#endif //__TIMER_H__
