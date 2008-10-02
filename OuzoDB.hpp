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

	template <class T>
	class BitmapAllocator
	{
	public:
		typedef size_t    size_type;
		typedef ptrdiff_t difference_type;
		typedef T*        pointer;
		typedef const T*  const_pointer;
		typedef T&        reference;
		typedef const T&  const_reference;
		typedef T         value_type;
	private:
		pointer m_ptr;
		size_type m_size;
	public:

		BitmapAllocator() {}
		BitmapAllocator(const BitmapAllocator& ba)
		{
			m_ptr=ba.m_ptr;
			m_size=ba.m_size;
		}

		pointer   allocate(size_type n, const void * = 0) 
		{
			T* t = (T*) malloc(n * sizeof(T));
			m_ptr=t;
			m_size=n*sizeof(T);
			return t;
		}
  
		void      deallocate(void* p, size_type) 
		{
			if (p)
			{
				free(p);
			} 
		}

		pointer           address(reference x) const { return &x; }
		const_pointer     address(const_reference x) const { return &x; }
		BitmapAllocator<T>&  operator=(const BitmapAllocator&) { return *this; }
		void              construct(pointer p, const T& val) { new ((T*) p) T(val); }
		void              destroy(pointer p) { p->~T(); }

		size_type         max_size() const { return size_t(-1); }

		template <class U>
		struct rebind { typedef BitmapAllocator<U> other; };

		template <class U>
		BitmapAllocator(const BitmapAllocator<U>&) {
		}

		template <class U>
		BitmapAllocator& operator=(const BitmapAllocator<U>& ba) { 
			m_ptr=ba.m_ptr;
			m_size=ba.m_size;
			return *this; 
		}

		size_type sizeInBytes() const { return m_size; }
		char* startOfSpace() { return (char*)m_ptr; }
	};
	
	class DocSet
	{
	public:
		typedef enum { arr, bitmap } set_type;
		typedef dynamic_bitset< unsigned long,BitmapAllocator<unsigned long> > bitset_type;
	private:
		// union DocUnion
		// {
			shared_ptr< vector<docid_t> > m_docs_arr;
			shared_ptr<bitset_type> m_docs_bitmap;
		// } m_docs;
		set_type m_type;
		
	public:
		DocSet(size_t capacity=1000);
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

	class Config
	{
		map<std::string,std::string> m_info;
	public:
		Config() {}
		~Config() {}
		
		inline std::string get(const std::string& s) const { return ((Config*)this)->m_info[s]; }
		void set(std::string name, std::string value);
		void set(std::string name, uint32_t value);
	};

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
		Config m_cfg;

		void addXMLDocument(bfs::path fname, docid_t docid);
		const char* getNodeValue(const DOMNode* node,const char* tag);
		void getValues(DOMDocument* document,const char*);
		void persist();
		
	public:	
		Ouzo(bfs::path config_file);
		~Ouzo();
		
		Config config() const { return m_cfg; }
	
		void addDocument(bfs::path docfile,doctype type=XML);
		void delDocument(bfs::path docfile);
	
		// Results fetch(const Query& q) const;
	};
	
	ostream& operator<<(ostream& os, const Ouzo& ouzo);
	
}
