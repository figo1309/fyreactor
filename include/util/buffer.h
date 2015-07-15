/************************************************************************/
/*
create time:	2015/6/10
athor:			¸ð·ÉÔ¾
discribe:		¼òµ¥»º³åÇø
*/
/************************************************************************/

#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "../net/define.hpp"

#define INIT_BUFFER_SIZE 1024			//1K

namespace fyreactor
{
	class CBuffer :public nocopyable
	{
	public:
		CBuffer();
		virtual ~CBuffer();

		int32_t AddBuf(const char* msg, uint32_t len);
		const char* PopBuf(uint32_t& len);

	private:
		uint32_t NextPowerOf2(uint32_t n) const;
		
	private:
		char*				m_pBuf;
		uint32_t			m_iSize;
		uint32_t			m_iLen;
	};



}

#endif //__BUFFER_H__
