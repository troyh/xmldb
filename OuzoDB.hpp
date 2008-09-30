#include <execinfo.h>
#include <iostream>
#include <map>
#include <vector>
#include <fstream>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/path.hpp>
#include <boost/serialization/vector.hpp>
// #include <boost/serialization/string.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <xercesc/framework/StdInInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMBuilder.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMWriter.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>

#include <xqilla/xqilla-dom3.hpp>


namespace Ouzo
{
	namespace bfs=boost::filesystem;
	using namespace std;
	using namespace boost;
	using namespace xercesc;
	
	typedef uint32_t docid_t;
	
	class Exception : public std::exception
	{
		const char* const m_f;
		size_t m_ln;
	public:
		Exception(const char* f,size_t ln) : m_f(f), m_ln(ln) {}
		virtual ~Exception() throw() {}
		
		const char* const file() const { return m_f; }
		size_t line() const { return m_ln; }
	};

	class Semaphore
	{
	public:
		Semaphore();
		~Semaphore();
	};

	// template<T>
	// class Key
	// {
	// 	typedef enum { str, n } key_type;
	// 	union KeyUnion
	// 	{
	// 		char* str;
	// 		uint32_t n;
	// 	} m_key;
	// 	key_type m_key_type;
	// 		
	// public:	
	// 	Key();
	// 	Key(std::string& s);
	// 	Key(uint32_t n);
	// 	~Key() {}
	// 	
	// 	bool operator<(const Key& k) const;
	// };
	
	class DocSet
	{
	public:
		typedef enum { arr, bitmap } set_type;
	private:
		// union DocUnion
		// {
			shared_ptr< vector<docid_t> > m_docs_arr;
			shared_ptr< dynamic_bitset<> > m_docs_bitmap;
		// } m_docs;
		set_type m_type;
		
	public:
		DocSet();
		DocSet(DocSet& ds);
		DocSet(const DocSet& ds);
		~DocSet() {}
		
		DocSet& operator=(DocSet& ds);
		DocSet& operator=(const DocSet& ds);
		
		set_type type() const { return m_type; }
		size_t size() const;
		
		void set(docid_t docno);
		void clr(docid_t docno);
		
		DocSet& operator|=(const DocSet& ds);
		DocSet& operator&=(const DocSet& ds);
		
		void load(istream& is);

		void save(ostream& os) const;
		
		friend ostream& operator<<(ostream& os, const DocSet& ds);
		
	};

	ostream& operator<<(ostream& os, const DocSet& ds);

	class Index
	{
	protected:
		static map<string, Index*> m_indexes;
		
		bfs::path m_filename;
		std::string m_keyspec;
	public:
		Index(bfs::path index_file, const std::string& keyspec);
		virtual ~Index();
		
		virtual size_t documentCount() const;
	
		const bfs::path& filename() const { return m_filename; }
		const std::string& keyspec() const { return m_keyspec; }

		virtual void load()=0;
		
		virtual Semaphore lock();
		virtual void unlock(const Semaphore& sem);
		
		virtual void save() const=0;
		
		virtual void merge(const Index& other);
		
		virtual void put(const char* key,docid_t docid)=0;
		virtual const DocSet& get(const char* key) const=0;
		virtual void del(docid_t docid)=0;
		
	};
	
	class StringIndex : public Index
	{
		map< std::string, DocSet > m_map;
	public:
		typedef map< std::string, DocSet >::iterator iterator_type;
		
		StringIndex(const bfs::path& index_file, const std::string& keyspec) : Index(index_file, keyspec) {}
		
		inline DocSet& operator[](const string& key) {return m_map[key]; }
		inline iterator_type begin() { return m_map.begin(); }
		inline iterator_type end() { return m_map.end(); }
	
		void put(const char* key,docid_t docid);
		const DocSet& get(const char* key) const;
		void del(docid_t docid);
		
		void load();
		void save() const;

		friend ostream& operator<<(ostream&,const StringIndex&);
	};

	ostream& operator<<(ostream& os, const StringIndex& idx);
	
	class UIntIndex : public Index
	{
		map< uint32_t, DocSet > m_map;
	public:
		typedef map< uint32_t, DocSet >::iterator iterator_type;

		UIntIndex(const bfs::path& index_file, const std::string& keyspec) : Index(index_file, keyspec) {}
		
		inline DocSet& operator[](uint32_t key) {return m_map[key]; }
		inline iterator_type begin() { return m_map.begin(); }
		inline iterator_type end() { return m_map.end(); }

		void put(const char* key,docid_t docid);
		const DocSet& get(const char* key) const { return get(strtoul(key,0,10)); }
		const DocSet& get(uint32_t key) const;
		void del(docid_t docid);

		void load();
		void save() const;
		
		friend ostream& operator<<(ostream&,const UIntIndex&);
	};

	ostream& operator<<(ostream& os, const UIntIndex& idx);

	// class InvertedIndex
	// {
	// 	typedef map< uint32_t, vector<Key> > index_type;
	// 	typedef map< uint32_t, vector<Key> >::iterator iterator_type;
	// public:
	// 	InvertedIndex();
	// 	~InvertedIndex();
	// };

	class Ouzo
	{
		friend ostream& operator<<(ostream& os, const Ouzo& ouzo);
	public:
		typedef enum { XML } doctype; // Supported document types
	private:
		map<bfs::path,docid_t> m_docidmap;
		dynamic_bitset<> m_avail_docids;
		vector<Index*> m_indexes;
		bfs::path m_config_file;
		bfs::path m_cfg_docdir;
		bfs::path m_cfg_datadir;
		uint32_t m_cfg_doccapacity;

		void addXMLDocument(bfs::path fname, docid_t docid);
		const char* getNodeValue(const DOMNode* node,const char* tag);
		void getValues(DOMDocument* document,const char*);
		void persist();
		
	public:	
		Ouzo(bfs::path config_file);
		~Ouzo();
	
		void addDocument(bfs::path docfile,doctype type=XML);
		void delDocument(bfs::path docfile);
	
		// Results fetch(const Query& q) const;
	};
	
	ostream& operator<<(ostream& os, const Ouzo& ouzo);
	
}
