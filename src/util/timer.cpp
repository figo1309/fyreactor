/************************************************************************/
/*
create time:	2015/6/5
athor:			���Ծ
discribe:		��ʱ��ͷ�ļ�
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
			//1.�ҵ����紥���Ķ�ʱ����ʱ��

			nextTime = -1;
			firstTimer.reset();

			//mark1�����������÷�
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

			//2.���û�ж�ʱ�����ȴ�һСʱ
			if (nextTime == -1)
			{
				//mark1�������condition_variable_anyʹ�ã�wait_forʱ�൱�ڽ�����
				m_condition.wait_for(lock, std::chrono::hours(1));
			}
			else
			{
				//3.�ж�ʱ����������ȼ���Ƿ��Ѿ���ʱ(10ms�ľ������)
				uint64 nowTime = GetMilSec();
				if (nowTime + 10 >= (uint64)nextTime)
				{
					//3.1ʱ���Ѿ����ˣ���ִ�ж�ʱ����
					if (m_outMutex != NULL)
					{
						//mark2:������һ�����������Ϊ��ȷ��m_outMutex -> m_mutexTimer�����ļ���˳����Ϊ������ԣ�ֻ����m_outMutex -> m_mutexTimer
						//�ļ���˳��������ﲻ������������������߳�����������������Ӷ���������
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

					//3.2һ���Զ�ʱ������ɾ������һ���Զ�ʱ�����¼��뷽������
					if (firstTimer->m_bOnce)
						firstTimer->m_bDeleted = true;
					else
					{
						firstTimer->m_iNextTime = nowTime + firstTimer->m_iInterval;

						m_mapOrderedTimers.erase(iterTimer);
						m_mapOrderedTimers.insert(std::make_pair(firstTimer->m_iNextTime, firstTimer));
					}

					//mark1���˳������������ڣ�����
					continue;
				}
				else
				{
					//3.3ʱ��û�е�����ִ�еȴ�
					//mark1�������condition_variable_anyʹ�ã�wait_forʱ�൱�ڽ�����
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
