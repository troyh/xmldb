#ifndef _OUZO_INDEX_HPP
#define _OUZO_INDEX_HPP

#include <iostream>
#include <map>

#include <boost/filesystem.hpp>

#include "DocSet.hpp"

namespace Ouzo
{
	using namespace std;
	namespace bfs=boost::filesystem;

	class Index
	{
		friend ostream& operator<<(ostream&,const Index&);
	public:
	
		static const uint32_t FILEVERSION=2;
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
		};
	
	protected:
		static std::map<std::string, Index*> m_indexes;

		uint32_t m_version;
		HeaderInfo m_headerinfo;
		bfs::path m_filename;
		std::string m_keyspec;

		void writeMeta(ostream& ofs) const;
		void readMeta(istream& ifs);
	public:
	
		Index(bfs::path index_file, const std::string& keyspec, uint32_t doccapacity);
		virtual ~Index();
	
		uint32_t version() const { return m_version; };
		virtual size_t documentCount() const { return m_headerinfo.doccount; }
		virtual size_t documentCapacity() const { return m_headerinfo.doccapacity; }
		virtual size_t keyCount() const=0;

		const bfs::path& filename() const { return m_filename; }
		const std::string& keyspec() const { return m_keyspec; }

		virtual void load()=0;
	
		virtual void save() const=0;
	
		virtual void merge(const Index& other);
	
		virtual void put(const char* key,docid_t docid)=0;
		virtual const DocSet& get(const char* key) const=0;
		virtual void del(docid_t docid)=0;
	
	};

	ostream& operator<<(ostream& os, const Index& idx);
}

#endif
