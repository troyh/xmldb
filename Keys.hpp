#include "Index.hpp"

namespace Ouzo
{

class int8key_t : public Index::key_t
{
public:
	int8key_t(const Index* idx, int8_t n=0) : Index::key_t(idx) { m_val.int8=n; }
	void assign(const char* s);
};

class int16key_t : public Index::key_t
{
public:
	int16key_t(const Index* idx, int16_t n=0) : Index::key_t(idx) { m_val.int16=n; }
	void assign(const char* s);
};

class int32key_t : public Index::key_t
{
public:
	int32key_t(const Index* idx, int32_t n=0) : Index::key_t(idx) { m_val.int32=n; }
	void assign(const char* s);
};

class int64key_t : public Index::key_t
{
public:
	int64key_t(const Index* idx, int64_t n=0) : Index::key_t(idx) { m_val.int64=n; }
	void assign(const char* s);
};
	
	
class uint8key_t : public Index::key_t
{
public:
	uint8key_t(const Index* idx, uint8_t n=0) : Index::key_t(idx) { m_val.uint8=n; }
	void assign(const char* s);
};

class uint16key_t : public Index::key_t
{
public:
	uint16key_t(const Index* idx, uint16_t n=0) : Index::key_t(idx) { m_val.uint16=n; }
	void assign(const char* s);
};

class uint32key_t : public Index::key_t
{
public:
	uint32key_t(const Index* idx, uint32_t n=0) : Index::key_t(idx) { m_val.uint32=n; }
	void assign(const char* s);
};

class uint64key_t : public Index::key_t
{
public:
	uint64key_t(const Index* idx, uint64_t n=0) : Index::key_t(idx) { m_val.uint64=n; }
	void assign(const char* s);
};

class doublekey_t : public Index::key_t
{
public:
	doublekey_t(const Index* idx, double n=0) : Index::key_t(idx) { m_val.dbl=n; }
	void assign(const char* s);
};

class char8key_t : public Index::key_t
{
public:
	char8key_t(const Index* idx, const char* s="") : Index::key_t(idx) { memcpy(m_val.ch,s,sizeof(m_val.ch)); }
};


class datekey_t : public Index::key_t
{
public:
	datekey_t(const Index* idx, uint32_t n) : Index::key_t(idx) { m_val.uint32=n; }
	
	void assign(time_t t);
	void assign(const char* s);
};

class timekey_t : public Index::key_t
{
public:	
	timekey_t(const Index* idx, time_t t) : Index::key_t(idx) { m_val.uint32=t; }

	void assign(time_t t);
	void assign(const char* s);
};

class floatkey_t : public Index::key_t
{
public:
	floatkey_t(const Index* idx, float f) : Index::key_t(idx) { m_val.dbl=f; }
	floatkey_t(const Index* idx, double d) : Index::key_t(idx) { m_val.dbl=d; }
	
	void assign(const char* s);
};

}
