/************************************************************************/
/*
create time:	2015/6/10
athor:			¸ð·ÉÔ¾
discribe:		¼òµ¥»º³åÇø
*/
/************************************************************************/

#include <util/buffer.h>

namespace fyreactor
{

	CBuffer::CBuffer()
		: m_pBuf(new char[INIT_BUFFER_SIZE])
		, m_iSize(INIT_BUFFER_SIZE)
		, m_iLen(0)
	{
	}

	CBuffer::CBuffer(const CBuffer&buffer)
	{
		m_iSize = buffer.m_iSize;
		m_iLen = buffer.m_iLen;
		m_pBuf = new char[m_iSize];

		memcpy(m_pBuf, buffer.m_pBuf, m_iLen);
	}

	CBuffer::~CBuffer()
	{
		if (m_pBuf != NULL)
		{
			delete[]m_pBuf;
			m_pBuf = NULL;
		}
	}

	CBuffer& CBuffer::operator =(const CBuffer& buffer)
	{
		m_iSize = buffer.m_iSize;
		m_iLen = buffer.m_iLen;
		m_pBuf = new char[m_iSize];

		memcpy(m_pBuf, buffer.m_pBuf, m_iLen);

		return *this;
	}

	int32_t CBuffer::AddBuf(const char* msg, uint32_t len)
	{
		if (m_iLen + len > MAX_MESSAGE_LEGNTH)
			return -1;

		if (m_iSize >= m_iLen + len)
		{
			memcpy(m_pBuf + m_iLen, msg, len);
			m_iLen += len;
		}
		else
		{
			m_iSize = NextPowerOf2(m_iLen + len);
			char* newBuf = new char[m_iSize];

			memcpy(newBuf, m_pBuf, m_iLen);
			memcpy(newBuf + m_iLen, msg, len);

			delete[]m_pBuf;
			m_pBuf = newBuf;
			m_iLen += len;
		}


		return (int32_t)m_iLen;
	}

	const char* CBuffer::PopBuf(uint32_t& len)
	{
		len = m_iLen;
		m_iLen = 0;
		return m_pBuf;
	}

	uint32_t CBuffer::NextPowerOf2(uint32_t n) const
	{
		/*double dLog2 = log2(n);
		uint32_t iLog2 = (uint32_t)dLog2 + 1;
		uint32_t ret = exp2(iLog2);

		return ret;*/

		if (n < INIT_BUFFER_SIZE)
			return INIT_BUFFER_SIZE;
		if (n < 2048)
			return 2048;
		if (n < 4096)
			return 4096;
		if (n < 8192)
			return 8192;
		if (n < 16384)
			return 16384;
		
		return MAX_MESSAGE_LEGNTH;
	}
}

