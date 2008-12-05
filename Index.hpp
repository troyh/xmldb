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
		
		class key_t
		{
		public:
			typedef enum 
			{
				KEY_TYPE_UNKNOWN =0,
				KEY_TYPE_INT8,
				KEY_TYPE_INT16,
				KEY_TYPE_INT32,
				KEY_TYPE_INT64,
				KEY_TYPE_UINT8,
				KEY_TYPE_UINT16,
				KEY_TYPE_UINT32, // 7
				KEY_TYPE_UINT64,
				KEY_TYPE_DBL,
				KEY_TYPE_CHAR8,
				KEY_TYPE_PTR,
				KEY_TYPE_DATE,
				KEY_TYPE_TIME,
				KEY_TYPE_FLOAT,
				KEY_TYPE_STRING,
				KEY_TYPE_OBJECT, // 16
				KEY_TYPE_LASTSTD=100
			} key_type;

		protected:
			key_type m_type;
			
		public:
			static key_type getKeyType(const char* kt);
			
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
				key_t*	 object;
			} m_val;
			
			key_t() : m_type(KEY_TYPE_UNKNOWN) { memset((char*)&m_val,0,sizeof(m_val)); }
			key_t(key_type t) : m_type(t) { memset((char*)&m_val,0,sizeof(m_val)); }
			key_t(const Index::key_t& k);
			
			virtual ~key_t() {}
			
			virtual key_t& operator=(const key_t& key) { m_type=key.m_type; m_val=key.m_val; return *this; }
			
			virtual void assign(int8_t   x) { m_type=KEY_TYPE_INT8  ; m_val.int8  =x; }
			virtual void assign(int16_t  x) { m_type=KEY_TYPE_INT16 ; m_val.int16 =x; }
			virtual void assign(int32_t  x) { m_type=KEY_TYPE_INT32 ; m_val.int32 =x; }
			virtual void assign(int64_t  x) { m_type=KEY_TYPE_INT64 ; m_val.int64 =x; }
			virtual void assign(uint8_t  x) { m_type=KEY_TYPE_UINT8 ; m_val.uint8 =x; }
			virtual void assign(uint16_t x) { m_type=KEY_TYPE_UINT16; m_val.uint16=x; }
			virtual void assign(uint32_t x) { m_type=KEY_TYPE_UINT32; m_val.uint32=x; }
			virtual void assign(uint64_t x) { m_type=KEY_TYPE_UINT64; m_val.uint64=x; }
			virtual void assign(double   x) { m_type=KEY_TYPE_DBL   ; m_val.dbl   =x; }
			virtual void assign(void*	 x) { m_type=KEY_TYPE_PTR   ; m_val.ptr   =x; }
			virtual void assign(key_t*   x) { m_type=KEY_TYPE_OBJECT; m_val.object=x; }
			virtual void assign(const char* x) { m_type=KEY_TYPE_CHAR8; strncpy(m_val.ch,x,sizeof(m_val.ch)); }

			virtual bool operator==(const key_t& k) const;
			virtual bool operator!=(const key_t& k) const { return !operator==(k); };
			
			virtual bool operator< (const key_t& key) const;

			// virtual operator char*() { return (char*)&m_val.ptr; }
			// virtual size_t size() const { return sizeof(m_val); }
			
			const key_type getType() const { return m_type; }
			
			virtual void output(ostream&) const;
			virtual void outputBinary(ostream&,uint32_t n) const;
			virtual void inputBinary(istream&);
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
			uint32_t doccapacity;
			uint16_t keyspeclen;
			uint32_t keycount;
			uint32_t keysize;
			key_t::key_type type;
		};


		void writeMeta(ostream& ofs) const;
		void readMeta(istream& ifs);
		
	protected:
		static std::map<std::string, Index*> m_indexes;

	public:
		typedef std::map< key_t, DocSet > map_type;
		typedef map_type::iterator map_iterator;
		typedef map_type::const_iterator const_map_iterator;
	protected:
		
		map_type m_map;
		VersionInfo m_verinfo;
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


		Index(const std::string& name, key_t::key_type kt, const std::string& keyspec, uint32_t doccapacity);
		virtual ~Index();
		
		uint32_t version() const { return m_verinfo.version; };
		virtual size_t documentCapacity() const { return m_headerinfo.doccapacity; }
		virtual size_t keyCount() const { return m_map.size(); }
		virtual key_t::key_type keyType() const { return m_headerinfo.type; }
		virtual key_t::key_type baseKeyType() const { return m_headerinfo.type; }

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
		
		virtual bool del(docid_t docid, Index::key_t* k=NULL);

		virtual map_type::iterator begin() 					 		 { return m_map.begin(); }
		virtual map_type::iterator end()   					 		 { return m_map.end();   }
		virtual map_type::iterator lower_bound(const key_t& key) 		 { return m_map.lower_bound(key); }

		virtual map_type::const_iterator begin() const					 		 { return m_map.begin(); }
		virtual map_type::const_iterator end()   const					 		 { return m_map.end();   }
		virtual map_type::const_iterator lower_bound(const key_t& key) const		 { return m_map.lower_bound(key); }
		
		// virtual Iterator* begin() 					 		 { return new Iterator(this,m_map.begin()); }
		// virtual Iterator* end()   					 		 { return new Iterator(this,m_map.end());   }
		// virtual Iterator* lower_bound(const key_t& key) 		 { return new Iterator(this,m_map.lower_bound(key)); }
		// 
		// virtual ConstIterator* begin() const					 		 { return new ConstIterator(this,m_map.begin()); }
		// virtual ConstIterator* end() const  					 		 { return new ConstIterator(this,m_map.end());   }
		// virtual ConstIterator* lower_bound(const key_t& key) const		 { return new ConstIterator(this,m_map.lower_bound(key)); }
		
		void setFilename(bfs::path fname);
		
		void initFile();
		
		virtual void output(ostream&) const;
		
	};

	ostream& operator<<(ostream& os, const Index& idx);
	ostream& operator<<(ostream& os, const Index::key_t& key);
	ostream& operator<<(ostream& os, Index::key_t::key_type t);

}

#endif
