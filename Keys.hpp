#include "Index.hpp"

namespace Ouzo
{

class int8key_t : public Index::key_t
{
public:
	int8key_t(int8_t n=0) : Index::key_t(KEY_TYPE_INT8) { m_val.int8=n; }
	void assign(const char* s);
};

class int16key_t : public Index::key_t
{
public:
	int16key_t(int16_t n=0) : Index::key_t(KEY_TYPE_INT16) { m_val.int16=n; }
	void assign(const char* s);
};

class int32key_t : public Index::key_t
{
public:
	int32key_t(int32_t n=0) : Index::key_t(KEY_TYPE_INT32) { m_val.int32=n; }
	void assign(const char* s);
};

class int64key_t : public Index::key_t
{
public:
	int64key_t(int64_t n=0) : Index::key_t(KEY_TYPE_INT64) { m_val.int64=n; }
	void assign(const char* s);
};
	
	
class uint8key_t : public Index::key_t
{
public:
	uint8key_t(uint8_t n=0) : Index::key_t(KEY_TYPE_UINT8) { m_val.uint8=n; }
	void assign(const char* s);
};

class uint16key_t : public Index::key_t
{
public:
	uint16key_t(uint16_t n=0) : Index::key_t(KEY_TYPE_UINT16) { m_val.uint16=n; }
	void assign(const char* s);
};

class uint32key_t : public Index::key_t
{
public:
	uint32key_t(uint32_t n=0) : Index::key_t(KEY_TYPE_UINT32) { m_val.uint32=n; }
	void assign(const char* s);
};

class uint64key_t : public Index::key_t
{
public:
	uint64key_t(uint64_t n=0) : Index::key_t(KEY_TYPE_UINT64) { m_val.uint64=n; }
	void assign(const char* s);
};

class doublekey_t : public Index::key_t
{
public:
	doublekey_t(double n=0) : Index::key_t(KEY_TYPE_DBL) { m_val.dbl=n; }
	void assign(const char* s);
};

class char8key_t : public Index::key_t
{
public:
	char8key_t(const char* s="") : Index::key_t(KEY_TYPE_CHAR8) { memcpy(m_val.ch,s,sizeof(m_val.ch)); }
};


class datekey_t : public Index::key_t
{
public:
	datekey_t(uint32_t n=0) : Index::key_t(KEY_TYPE_DATE) { m_val.uint32=n; }
	
	void assign(time_t t);
	void assign(const char* s);
};

class timekey_t : public Index::key_t
{
public:	
	timekey_t(time_t t=0) : Index::key_t(KEY_TYPE_TIME) { m_val.uint32=t; }

	void assign(time_t t);
	void assign(const char* s);
};

class floatkey_t : public Index::key_t
{
public:
	floatkey_t(float f=0) : Index::key_t(KEY_TYPE_DBL) { m_val.dbl=f; }
	
	void assign(const char* s);
};

}
