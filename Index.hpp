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

	public:
		Key() { m_val.uint64=0; }
		Key(const Key& key) { m_val=key.m_val; }
		Key(int8_t   x) { m_val.int8  =x; }
		Key(int16_t  x) { m_val.int16 =x; }
		Key(int32_t  x) { m_val.int32 =x; }
		Key(int64_t  x) { m_val.int64 =x; }
		Key(uint8_t  x) { m_val.uint8 =x; }
		Key(uint16_t x) { m_val.uint16=x; }
		Key(uint32_t x) { m_val.uint32=x; }
		Key(uint64_t x) { m_val.uint64=x; }
		Key(double   x) { m_val.dbl   =x; }
		Key(const char* x) { memcpy(m_val.ch,x,sizeof(m_val.ch)); }
		Key(void*	 x) { m_val.ptr   =x; } 
		
		Key& operator=(const Key& key) { m_val=key.m_val; return *this; }
		Key& operator=(int8_t   x) { m_val.int8  =x; return *this; }
		Key& operator=(int16_t  x) { m_val.int16 =x; return *this; }
		Key& operator=(int32_t  x) { m_val.int32 =x; return *this; }
		Key& operator=(int64_t  x) { m_val.int64 =x; return *this; }
		Key& operator=(uint8_t  x) { m_val.uint8 =x; return *this; }
		Key& operator=(uint16_t x) { m_val.uint16=x; return *this; }
		Key& operator=(uint32_t x) { m_val.uint32=x; return *this; }
		Key& operator=(uint64_t x) { m_val.uint64=x; return *this; }
		Key& operator=(double   x) { m_val.dbl   =x; return *this; }
		Key& operator=(char*    x) { memcpy(m_val.ch,x,sizeof(m_val.ch)); return *this; }
		Key& operator=(void*	x) { m_val.ptr   =x; return *this; } 
		
		operator       char*()       { return m_val.ch; }
		operator const char*() const { return (const char*)m_val.ch; }

		operator int8_t()   const { return m_val.int8;   }
		operator int16_t()  const { return m_val.int16;  }
		operator int32_t()  const { return m_val.int32;  }
		operator int64_t()  const { return m_val.int64;  }
		operator uint8_t()  const { return m_val.uint8;  }
		operator uint16_t() const { return m_val.uint16; }
		operator uint32_t() const { return m_val.uint32; }
		operator uint64_t() const { return m_val.uint64; }
		operator double()   const { return m_val.dbl;    }
		operator void*()    const { return m_val.ptr;    }
		
		       bool operator< (const Key& key) const;
		inline bool operator<=(const Key& key) const { return   m_val.uint64==key.m_val.uint64 || *this<key; }
		inline bool operator> (const Key& key) const { return !(m_val.uint64==key.m_val.uint64 || *this<key); }
		inline bool operator>=(const Key& key) const { return !(*this<key); }
		inline bool operator==(const Key& key) const { return m_val.uint64==key.m_val.uint64; }
		inline bool operator!=(const Key& key) const { return m_val.uint64!=key.m_val.uint64; }
	};
	
	class Index
	{
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
			// index_type type;
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
		
		typedef uint32_t indexid_t;
		// typedef uint32_t lookupid_t;
		// static const lookupid_t lookupid_t_nil=static_cast<lookupid_t>(-1);
		
		// typedef enum 
		// {
		// 	INDEX_TYPE_UNKNOWN	=0, 
		// 	INDEX_TYPE_STRING	=1, 
		// 	INDEX_TYPE_UINT8	=2, 
		// 	INDEX_TYPE_UINT16	=3, 
		// 	INDEX_TYPE_UINT32	=4, 
		// 	INDEX_TYPE_FLOAT	=5,
		// 	INDEX_TYPE_DATE		=6,
		// 	INDEX_TYPE_TIME		=7,
		// 	INDEX_TYPE_SINT8	=8, 
		// 	INDEX_TYPE_SINT16	=9, 
		// 	INDEX_TYPE_SINT32	=10
		// } index_type;

		// static index_type getType(const Index& idx);
		// static Index* getIndexFromID(indexid_t n) { return (Index*)n; }
		// static Index* loadIndexFromFile(bfs::path filename);
		

		class Iterator
		{
			Index* m_idx;
			
		public:
			Iterator(const Iterator&);
			Iterator& operator=(const Iterator&);
			Iterator(Index* idx) : m_idx(idx) {}
			~Iterator() {}
			
			bool operator==(const Iterator& itr);
			bool operator!=(const Iterator& itr);
			
			Iterator& operator++();
			Iterator& operator++(int);
			
			Key key() const;
			DocSet& docset();
		};
		
	
		Index(const std::string& name, bfs::path index_file, const std::string& keyspec, uint32_t doccapacity);
		virtual ~Index();
		
		uint32_t version() const { return m_version; };
		virtual size_t documentCount() const { return m_headerinfo.doccount; }
		virtual size_t documentCapacity() const { return m_headerinfo.doccapacity; }
		virtual size_t keyCount() const { return m_map.size(); }

		const std::string& name() const 				{ return m_name; }
		const bfs::path&   filename() const 			{ return m_filename; }
		const std::string& keyspec() const 				{ return m_keyspec; }
		DocumentBase* 	   getDocBase() const 			{ return m_db; }
		void 			   setDocBase(DocumentBase* p) 	{ m_db=p; }

		virtual void load();
		virtual void save() const;
	
		virtual void put(const Key&	key, docid_t docid);
		
		virtual       DocSet& get(const Key& key);
		virtual const DocSet& get(const Key& key) const;
		
		virtual void del(docid_t docid);
		
		Iterator			begin();// 		{ return m_map.begin(); }
		Iterator			end();// 	 		{ return m_map.end(); 	}

		Iterator       lower_bound(const Key& key);// 		{ return m_map.lower_bound(key); }
		
		virtual void output(ostream&) const;
		
	};

	ostream& operator<<(ostream& os, const Index& idx);

	class DateIndex : public Index
	{
	public:	
		DateIndex(const std::string& name, bfs::path index_file, const std::string& keyspec, uint32_t doccapacity)
			: Index(name,index_file,keyspec,doccapacity) {}
		~DateIndex() {}
		
		void output(ostream&) const;
		
	};
	
	class TimeIndex : public Index
	{
	public:	
		TimeIndex(const std::string& name, bfs::path index_file, const std::string& keyspec, uint32_t doccapacity)
			: Index(name,index_file,keyspec,doccapacity) {}
		~TimeIndex() {}

		void output(ostream&) const;
	};

	class FloatIndex : public Index
	{
	public:	
		FloatIndex(const std::string& name, bfs::path index_file, const std::string& keyspec, uint32_t doccapacity)
			: Index(name,index_file,keyspec,doccapacity) {}
		~FloatIndex() {}

		void output(ostream&) const;
	};
	
}

#endif
