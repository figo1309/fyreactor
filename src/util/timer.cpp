/************************************************************************/
/*
create time:	2015/6/5
athor:			葛飞跃
discribe:		定时器头文件
*/
/************************************************************************/

#include <util/timer.h>
#include <ctime>
#include <sys/types.h>
#include <sys/timeb.h> 
#include <time.h>

namespace fyreactor
{

	CTimerThread::CTimerThread(std::recursive_mutex* mutex)
		: m_outMutex(mutex)
		, m_iCurretId(0)
		, m_bRun(true)
		, m_pThread(NULL)
	{

	}

	CTimerThread::~CTimerThread()
	{
		if (m_pThread != NULL)
		{
			m_pThread->join();
			delete m_pThread;
			m_pThread = NULL;
		}
	}

	uint64 CTimerThread::Register(bool run_once, int interval_time, std::function<void(long)> fp)
	{
		std::shared_ptr<STimer> newTimer = std::make_shared<STimer>();
		newTimer->m_iId = ++m_iCurretId;
		newTimer->m_bOnce = run_once;
		newTimer->m_pFunc = fp;
		newTimer->m_iInterval = interval_time * 1000;
		newTimer->m_iStartTime = GetMilSec();
		newTimer->m_iNextTime = newTimer->m_iStartTime + newTimer->m_iInterval;

		{
			std::unique_lock<std::recursive_mutex>	lock(m_mutexTimer);
			m_mapOrderedTimers.insert(std::make_pair(newTimer->m_iNextTime, newTimer));
			m_mapTimers[newTimer->m_iId] = newTimer;

			m_condition.notify_one();
		}

		return newTimer->m_iId;
	}

	uint64 CTimerThread::RegisterByMilSec(bool run_once, int interval_time, std::function<void(long)> fp)
	{
		std::shared_ptr<STimer> newTimer = std::make_shared<STimer>();
		newTimer->m_iId = ++m_iCurretId;
		newTimer->m_bOnce = run_once;
		newTimer->m_pFunc = fp;
		newTimer->m_iInterval = interval_time;
		newTimer->m_iStartTime = GetMilSec();
		newTimer->m_iNextTime = newTimer->m_iStartTime + newTimer->m_iInterval;

		{
			std::unique_lock<std::recursive_mutex>	lock(m_mutexTimer);
			m_mapOrderedTimers.insert(std::make_pair(newTimer->m_iNextTime, newTimer));
			m_mapTimers[newTimer->m_iId] = newTimer;

			m_condition.notify_one();
		}

		return newTimer->m_iId;
	}

	void CTimerThread::UnRegister(uint64 timer_id)
	{
		std::unique_lock<std::recursive_mutex>	lock(m_mutexTimer);

		auto findIter = m_mapTimers.find(timer_id);
		if (findIter != m_mapTimers.end())
		{
			findIter->second->m_bDeleted = true;
		}
	}

	void CTimerThread::Run()
	{
		m_pThread = new std::thread(std::bind(&CTimerThread::RunInThead, this));
	}

	void CTimerThread::RunInThead()
	{
		int64 nextTime;
		std::shared_ptr<STimer> firstTimer;
		do {
			//1.找到最早触发的定时器的时间

			nextTime = -1;
			firstTimer.reset();

			//mark1：这里锁的用法
			std::unique_lock<std::recursive_mutex>	lock(m_mutexTimer);

			auto iterTimer = m_mapOrderedTimers.begin();
			for (; iterTimer != m_mapOrderedTimers.end();)
			{
				if (iterTimer->second->m_bDeleted == true)
				{
					m_mapTimers.erase(iterTimer->second->m_iId);
					iterTimer = m_mapOrderedTimers.erase(iterTimer);
				}
				else
				{
					nextTime = iterTimer->second->m_iNextTime;
					firstTimer = iterTimer->second;
					break;
				}
			}

			//2.如果没有定时器，等待一小时
			if (nextTime == -1)
			{
				//mark1：锁配合condition_variable_any使用，wait_for时相当于解锁了
				m_condition.wait_for(lock, std::chrono::hours(1));
			}
			else
			{
				//3.有定时器的情况，先检查是否已经超时(10ms的精度误差)
				uint64 nowTime = GetMilSec();
				if (nowTime + 10 >= (uint64)nextTime)
				{
					//3.1时间已经到了，则执行定时函数
					if (m_outMutex != NULL)
					{
						//mark2:这里有一点特殊操作，为了确保m_outMutex -> m_mutexTimer这样的加锁顺序，因为对外而言，只能是m_outMutex -> m_mutexTimer
						//的加锁顺序，如果这里不这样做，会出现两个线程两个锁反序加锁，从而导致死锁
						lock.unlock();
						std::unique_lock<std::recursive_mutex> lock1(*m_outMutex);
						lock.lock();
						if (!firstTimer->m_bDeleted)
							firstTimer->m_pFunc(nowTime);
					}
					else
					{
						firstTimer->m_pFunc(nowTime);
					}

					//3.2一次性定时器则标记删除，非一次性定时器重新加入方便排序
					if (firstTimer->m_bOnce)
						firstTimer->m_bDeleted = true;
					else
					{
						firstTimer->m_iNextTime = nowTime + firstTimer->m_iInterval;

						m_mapOrderedTimers.erase(iterTimer);
						m_mapOrderedTimers.insert(std::make_pair(firstTimer->m_iNextTime, firstTimer));
					}

					//mark1：退出锁的生命周期，解锁
					continue;
				}
				else
				{
					//3.3时间没有到，则执行等待
					//mark1：锁配合condition_variable_any使用，wait_for时相当于解锁了
					m_condition.wait_for(lock, std::chrono::milliseconds(nextTime - nowTime));
				}
			}

		} while (m_bRun);
	}

	void CTimerThread::Stop()
	{
		m_bRun = false;
		{
			std::unique_lock<std::recursive_mutex> lock(m_mutexTimer);
			m_condition.notify_one();
		}

		m_pThread->join();
		delete m_pThread;
		m_pThread = NULL;
	}

	uint64_t CTimerThread::GetMilSec()
	{
#ifdef HAVE_WINDOWS
		struct _timeb tb;
		_ftime_s(&tb);
		return tb.time * 1000 + tb.millitm;
#elif defined HAVE_LINUX
		struct timeb tb;
		ftime(&tb);
		return tb.time * 1000 + tb.millitm;
#endif
	}

	uint64_t CTimerThread::GetSec()
	{
		return  std::time(NULL);
	}
}
