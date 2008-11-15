#ifndef _OUZO_INDEX_HPP
#define _OUZO_INDEX_HPP

#include <iostream>
#include <map>
#include <string.h>

#include <boost/filesystem.hpp>

#include "DocSet.hpp"

namespace Ouzo
{
	using namespace std;
	namespace bfs=boost::filesystem;
	
	class DocumentBase;
	
	class Key
	{
	protected:
		union
		{
			int8_t   int8;
			int16_t  int16;
			int32_t  int32;
			int64_t  int64;
			uint8_t  uint8;
			uint16_t uint16;
			uint32_t uint32;
			uint64_t uint64;
			double   dbl;
			char     ch[8];
			void*	 ptr;
		} m_val;

		enum
		{
			KEY_TYPE_INT8 = 1,
			KEY_TYPE_INT16 =2,
			KEY_TYPE_INT32 =3,
			KEY_TYPE_INT64 =4,
			KEY_TYPE_UINT8 =5,
			KEY_TYPE_UINT16=6,
			KEY_TYPE_UINT32=7,
			KEY_TYPE_UINT64=8,
			KEY_TYPE_DBL   =9,
			KEY_TYPE_CH    =10,
			KEY_TYPE_PTR   =11
		} m_type;
		
	public:
		Key(const Key& key) : m_type(key.m_type     ) { m_val=key.m_val; }
		Key(const Key* key) : m_type(key->m_type    ) { m_val=key->m_val; }
		Key()               : m_type(KEY_TYPE_UINT64) { m_val.uint64=0;  }
		Key(int8_t   x)     : m_type(KEY_TYPE_INT8  ) { m_val.int8  =x;  }
		Key(int16_t  x)     : m_type(KEY_TYPE_INT16 ) { m_val.int16 =x;  }
		Key(int32_t  x)     : m_type(KEY_TYPE_INT32 ) { m_val.int32 =x;  }
		Key(int64_t  x)     : m_type(KEY_TYPE_INT64 ) { m_val.int64 =x;  }
		Key(uint8_t  x)     : m_type(KEY_TYPE_UINT8 ) { m_val.uint8 =x;  }
		Key(uint16_t x)     : m_type(KEY_TYPE_UINT16) { m_val.uint16=x;  }
		Key(uint32_t x)     : m_type(KEY_TYPE_UINT32) { m_val.uint32=x;  }
		Key(uint64_t x)     : m_type(KEY_TYPE_UINT64) { m_val.uint64=x;  }
		Key(double   x)     : m_type(KEY_TYPE_DBL   ) { m_val.dbl   =x;  }
		Key(const char* x)  : m_type(KEY_TYPE_CH    ) { memcpy(m_val.ch,x,sizeof(m_val.ch)); }
		Key(void*	 x)     : m_type(KEY_TYPE_PTR   ) { m_val.ptr   =x;  } 
		
		virtual ~Key() {}
		
		int getType() const { return m_type; }
		
		virtual Key& operator=(const Key& key) { m_type=key.m_type; m_val=key.m_val; return *this; }
		virtual Key& operator=(int8_t   x)     ;//{ m_type=KEY_TYPE_INT8  ; m_val.int8  =x;  return *this; }
		virtual Key& operator=(int16_t  x)     ;//{ m_type=KEY_TYPE_INT16 ; m_val.int16 =x;  return *this; }
		virtual Key& operator=(int32_t  x)     ;//{ m_type=KEY_TYPE_INT32 ; m_val.int32 =x;  return *this; }
		virtual Key& operator=(int64_t  x)     ;//{ m_type=KEY_TYPE_INT64 ; m_val.int64 =x;  return *this; }
		virtual Key& operator=(uint8_t  x)     ;//{ m_type=KEY_TYPE_UINT8 ; m_val.uint8 =x;  return *this; }
		virtual Key& operator=(uint16_t x)     ;//{ m_type=KEY_TYPE_UINT16; m_val.uint16=x;  return *this; }
		virtual Key& operator=(uint32_t x)     ;//{ m_type=KEY_TYPE_UINT32; m_val.uint32=x;  return *this; }
		virtual Key& operator=(uint64_t x)     ;//{ m_type=KEY_TYPE_UINT64; m_val.uint64=x;  return *this; }
		virtual Key& operator=(double   x)     ;//{ m_type=KEY_TYPE_DBL   ; m_val.dbl   =x;  return *this; }
		virtual Key& operator=(const char*    x)     ;//{ m_type=KEY_TYPE_CH    ; memcpy(m_val.ch,x,sizeof(m_val.ch)); return *this; }
		virtual Key& operator=(void*	x)     ;//{ m_type=KEY_TYPE_PTR   ; m_val.ptr   =x;  return *this; } 
		
		virtual operator       char*()       { return m_val.ch; }
		virtual operator const char*() const { return (const char*)m_val.ch; }

		virtual operator int8_t()   const { return m_val.int8;   }
		virtual operator int16_t()  const { return m_val.int16;  }
		virtual operator int32_t()  const { return m_val.int32;  }
		virtual operator int64_t()  const { return m_val.int64;  }
		virtual operator uint8_t()  const { return m_val.uint8;  }
		virtual operator uint16_t() const { return m_val.uint16; }
		virtual operator uint32_t() const { return m_val.uint32; }
		virtual operator uint64_t() const { return m_val.uint64; }
		virtual operator double()   const { return m_val.dbl;    }
		virtual operator void*()    const { return m_val.ptr;    }
		
		       virtual bool operator< (const Key& key) const;
		inline virtual bool operator<=(const Key& key) const { return *this==key || *this<key; }
		inline virtual bool operator> (const Key& key) const { return !(m_val.uint64==key.m_val.uint64 || *this<key); }
		inline virtual bool operator>=(const Key& key) const { return !(*this<key); }
		inline virtual bool operator==(const Key& key) const { return m_val.uint64==key.m_val.uint64; }
		inline virtual bool operator!=(const Key& key) const { return m_val.uint64!=key.m_val.uint64; }
		
		virtual void output(ostream& os) const;// { os << m_val.uint64; }
	};
	
	inline ostream& operator<<(ostream& os, const Key& key) { key.output(os); return os; }
	

	
	class Index
	{
	public:
		typedef char key_type[32];
	private:
		static const uint32_t FILEVERSION=3;
		struct VersionInfo
		{
			uint32_t version;
			uint32_t metasize;
		};
		
		struct HeaderInfo
		{
			uint32_t doccount;
			uint32_t doccapacity;
			uint16_t keyspeclen;
			uint32_t keycount;
			uint32_t keysize;
			char     type[32];
		};
		
		
		typedef std::map< Key, DocSet > map_type;

		void writeMeta(ostream& ofs) const;
		void readMeta(istream& ifs);
		
	protected:
		static std::map<std::string, Index*> m_indexes;

		typedef map_type::iterator map_iterator;
		typedef map_type::const_iterator const_map_iterator;
		
		map_type m_map;
		uint32_t m_version;
		HeaderInfo m_headerinfo;
		bfs::path m_filename;
		std::string m_keyspec;
		std::string m_name;
		DocumentBase* m_db;

		void load_data(istream& ifs);
		void save_data(ostream& ifs) const;

	public:
		static DocSet nil_docset;
		
		static Index* loadFromFile(bfs::path filename);

		class Iterator
		{
			Index* m_idx;
			Index::map_type::iterator m_itr;
			
		public:
			Iterator(const Iterator& itr) : m_idx(itr.m_idx), m_itr(itr.m_itr) {}
			Iterator& operator=(const Iterator& itr) { m_idx=itr.m_idx; m_itr=itr.m_itr; return *this; }
			Iterator(Index* idx, Index::map_type::iterator itr) : m_idx(idx), m_itr(itr) {}
			~Iterator() {}
			
			bool operator==(const Iterator& itr) { return m_itr==itr.m_itr; }
			bool operator!=(const Iterator& itr) { return m_itr!=itr.m_itr; }
			
			Iterator& operator++()    { m_itr++; return *this; }
			Iterator& operator++(int) { m_itr++; return *this; }
			
			Key key() const  { return m_itr->first; }
			DocSet& docset() { return m_itr->second; }
		};
		
	
		Index(const std::string& name, const key_type kt, const std::string& keyspec, uint32_t doccapacity);
		virtual ~Index();
		
		uint32_t version() const { return m_version; };
		virtual size_t documentCount() const { return m_headerinfo.doccount; }
		virtual size_t documentCapacity() const { return m_headerinfo.doccapacity; }
		virtual size_t keyCount() const { return m_map.size(); }
		virtual const char* keyType() const { return m_headerinfo.type; }

		const std::string& name() const 				{ return m_name; }
		const bfs::path&   filename() const 			{ return m_filename; }
		const std::string& keyspec() const 				{ return m_keyspec; }
		DocumentBase* 	   getDocBase() const 			{ return m_db; }
		void 			   setDocBase(DocumentBase* p) 	{ m_db=p; }

		virtual void load();
		virtual void save() const;
	
		virtual void put(const char* s, docid_t docid);
		virtual void put(const Key&	key, docid_t docid);
		
		virtual       DocSet& get(const Key& key);
		virtual const DocSet& get(const Key& key) const;
		
		virtual void del(docid_t docid);
		
		      Iterator begin() 					 		 { return Iterator(this,m_map.begin()); }
		// const Iterator begin() const		 			 { return Iterator(this,m_map.begin()); }
		      Iterator end()   					 		 { return Iterator(this,m_map.end());   }
		// const Iterator end() const  					 { return Iterator(this,m_map.end());   }
		      Iterator lower_bound(const Key& key) 		 { return Iterator(this,m_map.lower_bound(key)); }
		// const Iterator lower_bound(const Key& key) const { return Iterator(this,m_map.lower_bound(key)); }
		
		void setFilename(bfs::path fname) { m_filename=bfs::change_extension(fname,".index");  }
		
		virtual void output(ostream&) const;
		
	};

	ostream& operator<<(ostream& os, const Index& idx);

	class DateKey : public Key
	{
		uint32_t normalizeNoon(time_t);
	public:	
		DateKey() : Key() {}
		DateKey(time_t t) : Key(normalizeNoon(t)) {}
		~DateKey() {}
		
		Key& operator=(time_t t) { m_val.uint32=normalizeNoon(t); return *this; } 

		operator time_t()   const { return (time_t)m_val.uint32; }

		void output(ostream& os) const;
	};
	
	inline ostream& operator<<(ostream& os, const DateKey& key) { key.output(os); return os; }


	class TimeKey : public Key
	{
	public:	
		TimeKey() : Key() {}
		TimeKey(time_t t) : Key((uint32_t)t) {}
		~TimeKey() {}
		
		Key& operator=(time_t t) { m_val.uint32=(uint32_t)t; return *this; } 

		operator time_t()   const { return (time_t)m_val.uint32; }

		void output(ostream& os) const;
	};

	inline ostream& operator<<(ostream& os, const TimeKey& key) { key.output(os); return os; }


	class FloatKey : public Key
	{
	public:	
		FloatKey() : Key() {}
		FloatKey(double n) : Key(n) {}
		FloatKey(float n) : Key((double)n) {}
		~FloatKey() {}
		
		Key& operator=(double n) { m_val.dbl=n; return *this; } 
		Key& operator=(float n) { m_val.dbl=n; return *this; } 

		operator double()   const { return m_val.dbl; }
		operator float()   const { return m_val.dbl; }

		void output(ostream& os) const;
	};

	inline ostream& operator<<(ostream& os, const FloatKey& key) { key.output(os); return os; }

	class StringKey : public Key
	{
		std::string m_s;
	public:	
		StringKey() : Key() {}
		StringKey(const char* s) : Key(0), m_s(s) {}
		~StringKey() {}
		
		Key& operator=(const char* s) { m_s=s; return *this; } 

		operator const char*()   const { return m_s.c_str(); }

		void output(ostream& os) const;
	};

	inline ostream& operator<<(ostream& os, const StringKey& key) { key.output(os); return os; }
	
}

#endif
