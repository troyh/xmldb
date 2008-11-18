#ifndef _OUZO_KEYS_HPP
#define _OUZO_KEYS_HPP

#include "Keys.hpp"

namespace Ouzo
{

void int8key_t::assign(const char* s){m_val.int8=strtoul(s,0,10);}
void int16key_t::assign(const char* s){m_val.int16=strtoul(s,0,10);}
void int32key_t::assign(const char* s){m_val.int32=strtoul(s,0,10);}
void int64key_t::assign(const char* s){m_val.int64=strtoul(s,0,10);}
void uint8key_t::assign(const char* s){m_val.uint8=strtol(s,0,10);}
void uint16key_t::assign(const char* s){m_val.uint16=strtoul(s,0,10);}
void uint32key_t::assign(const char* s){m_val.uint32=strtoul(s,0,10);}
void uint64key_t::assign(const char* s){m_val.uint64=strtoul(s,0,10);}
void doublekey_t::assign(const char* s){m_val.dbl=strtod(s,0);}

void datekey_t::assign(time_t t)
{
	struct tm* ptm=localtime(&t);
	m_val.uint32=((ptm->tm_year+1900)*10000) + (ptm->tm_mon*100) + ptm->tm_mday;
}

void datekey_t::assign(const char* s)
{
	// TODO: implement this
}

void timekey_t::assign(time_t t)
{
	m_val.uint32=t;
}

void timekey_t::assign(const char* s)
{
	// TODO: implement this
}

void floatkey_t::assign(const char* s)
{
	m_val.dbl=strtod(s,0);
}

}


#endif
