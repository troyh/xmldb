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
	
	
	class Index
	{
	public:
		typedef char key_type[32];
		
		class key_t
		{
		protected:
			const Index* m_idx;
			
		public:
			
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
			
			key_t() : m_idx(0) { m_val.ptr=0; }
			key_t(const Index* idx) : m_idx(idx) { m_val.ptr=0; }
			key_t(const Index::key_t& k);
			
			virtual ~key_t() {}
			
			virtual key_t& operator=(const key_t& key) { m_idx=key.m_idx; m_val=key.m_val; return *this; }
			virtual void assign(int8_t   x) { m_val.int8  =x; }
			virtual void assign(int16_t  x) { m_val.int16 =x; }
			virtual void assign(int32_t  x) { m_val.int32 =x; }
			virtual void assign(int64_t  x) { m_val.int64 =x; }
			virtual void assign(uint8_t  x) { m_val.uint8 =x; }
			virtual void assign(uint16_t x) { m_val.uint16=x; }
			virtual void assign(uint32_t x) { m_val.uint32=x; }
			virtual void assign(uint64_t x) { m_val.uint64=x; }
			virtual void assign(double   x) { m_val.dbl   =x; }
			virtual void assign(const char* x) { memcpy(m_val.ch,x,sizeof(m_val.ch)); }
			virtual void assign(void*	x)  { m_val.ptr=x; }

			virtual bool operator==(const key_t& k) const;
			virtual bool operator!=(const key_t& k) const { return !operator==(k); };
			
			virtual bool operator< (const key_t& key) const;

			virtual operator char*() { return (char*)&m_val.ptr; }
			virtual size_t size() const { return sizeof(m_val); }
			
			const Index* getIndex() const { return m_idx; }
			
			virtual void output(ostream&) const;
		};
		
	
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

		typedef std::map< key_t, DocSet > map_type;

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
		protected:
			Index* m_idx;
		private:
			Index::map_type::iterator m_itr;
			
		public:
			Iterator(Index* idx) : m_idx(idx) {}
			Iterator(const Iterator& itr) : m_idx(itr.m_idx), m_itr(itr.m_itr) {}
			Iterator& operator=(const Iterator& itr) { m_idx=itr.m_idx; m_itr=itr.m_itr; return *this; }
			Iterator(Index* idx, Index::map_type::iterator itr) : m_idx(idx), m_itr(itr) {}
			~Iterator() {}
			
			bool operator==(const Iterator& itr) { return m_itr==itr.m_itr; }
			bool operator!=(const Iterator& itr) { return m_itr!=itr.m_itr; }
			
			Iterator& operator++()    { m_itr++; return *this; }
			Iterator& operator++(int) { m_itr++; return *this; }
			
			Index::key_t key() const  { return Index::key_t(m_itr->first); }
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
	
		virtual void put(const key_t&	key, docid_t docid);
		
		virtual key_t* createKey() const;
		
		virtual       DocSet& get(const key_t& key);
		virtual const DocSet& get(const key_t& key) const;
		
		virtual void del(docid_t docid);
		
		virtual Iterator* begin() 					 		 { return new Iterator(this,m_map.begin()); }
		virtual Iterator* end()   					 		 { return new Iterator(this,m_map.end());   }
		virtual Iterator* lower_bound(const key_t& key) 		 { return new Iterator(this,m_map.lower_bound(key)); }
		
		void setFilename(bfs::path fname) { m_filename=bfs::change_extension(fname,".index");  }
		
		virtual int compareKeys(const key_t& key1, const key_t& key2) const;
		
		virtual void output(ostream&) const;
		
	};

	ostream& operator<<(ostream& os, const Index& idx);
	ostream& operator<<(ostream& os, const Index::key_t& key);

}

#endif
