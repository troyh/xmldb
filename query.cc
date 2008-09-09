#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/filesystem.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/progress.hpp>

using namespace std;
namespace bfs=boost::filesystem;


namespace optionalcorp
{
	class Index
	{
		typedef map< string, vector<uint32_t> > index_type;
		
		index_type m_idx;
		
	public:
		Index(bfs::path index_file);
		
		inline vector<uint32_t>& operator[](string key) 
		{
			return m_idx[key];
		}
		
		size_t documentCount() const;
	};
	
	
	class QueryNode
	{
	protected:
		QueryNode* m_left;
		QueryNode* m_right;
	public:
		QueryNode() : m_left(0), m_right(0) {}
		virtual ~QueryNode() { delete m_left; delete m_right; }
		
		virtual void left(QueryNode* p){m_left=p;}
		virtual QueryNode* left(){return m_left;}
		
		virtual void right(QueryNode* p){m_right=p;}
		virtual QueryNode* right(){return m_right;}
		
		virtual bool isTermNode() const=0;
		virtual bool isOperNode() const=0;
		
		virtual size_t countOfTermNodes() const;
		virtual void getResult(boost::dynamic_bitset<>& result)=0;
	};
	
	class BooleanQueryNode: public QueryNode
	{
	public:
		typedef enum { OR, AND, NOT } boolean_type;
	protected:
		boolean_type m_type;
	public:
		BooleanQueryNode(boolean_type type) : m_type(type) {}
		
		bool isTermNode() const { return false; }
		bool isOperNode() const { return true; }

		void getResult(boost::dynamic_bitset<>& result);
	};
	
	class TermQueryNode : public QueryNode
	{
		string m_term;
		Index& m_idx;
	public:
		TermQueryNode(string& str, Index& idx) : m_term(str), m_idx(idx) {}
		
		bool isTermNode() const { return true; }
		bool isOperNode() const { return false; }

		void getResult(boost::dynamic_bitset<>& result);
	};

	class IndexQuery
	{
	public:
		
		typedef enum { OR, AND, NOT } boolean_type;
		
	private:
		
 		vector<string> m_terms;
		boolean_type m_boolop;
		
	public:	
		typedef vector<string>::size_type size_type;
		
		IndexQuery(boolean_type type);
		
		IndexQuery& add(const string& term);
		IndexQuery& add(const vector<string>& terms);
		
		const string& term(size_type n) const;
		
		size_type termCount() const;
	};
	
}

namespace optionalcorp
{
	size_t QueryNode::countOfTermNodes() const
	{
		size_t n=0;
		if (m_left)
		{
			if (m_left->isTermNode())
				n++;
			else
				n+=m_left->countOfTermNodes();
		}
		if (m_right)
		{
			if (m_right->isTermNode())
				n++;
			else
				n+=m_right->countOfTermNodes();
		}
		return n;
	}
	
	void TermQueryNode::getResult(boost::dynamic_bitset<>& result)
	{
		result.reset();
		
		for(size_t j = 0, jj=m_idx[m_term].size(); j < jj; ++j)
		{
			boost::dynamic_bitset<>::size_type bitno=(m_idx[m_term])[j]-1;
			result.set(bitno);
		}
		
	}

	void BooleanQueryNode::getResult(boost::dynamic_bitset<>& result)
	{
		// we have 2 children, get query results from both children
		boost::dynamic_bitset<> leftresult(result.size());
		m_left->getResult(leftresult);
		
		boost::dynamic_bitset<> rightresult(result.size());
		m_right->getResult(rightresult);
		
		switch (m_type)
		{
			case AND:
				result=leftresult & rightresult;
				break;
			case OR:
				result=leftresult | rightresult;
				break;
			case NOT:
				result=leftresult ^ rightresult;
				break;
			default:
				break;
		}
		
	}

	
	IndexQuery::IndexQuery(boolean_type type)
		: m_boolop(type)
	{
	}
	
	IndexQuery& IndexQuery::add(const string& term)
	{
		m_terms.push_back(term);
		return *this;
	}
	
	IndexQuery& IndexQuery::add(const vector<string>& terms)
	{
		for(vector<string>::size_type i = 0; i < terms.size(); ++i)
		{
			add(terms[i]);
		}
		return *this;
	}
	
	const string& IndexQuery::term(size_type n) const
	{
		return m_terms[n];
	}
	
	IndexQuery::size_type IndexQuery::termCount() const
	{
		return m_terms.size();
	}
	
	Index::Index(bfs::path index_file)
	{
		std::ifstream ifs(index_file.string().c_str());
		boost::archive::text_iarchive ar(ifs);
		boost::serialization::load(ar,m_idx,0);
	}
	
	size_t Index::documentCount() const
	{
		return 30000; // TODO: figure out actual value
	}
	
}

int main(int argc,char* argv[])
{
	boost::progress_timer query_timer;

	bfs::path idxfile(argv[1]);

	vector<string> terms;
	for (int i=2;i<argc;++i)
	{
		terms.push_back(argv[i]);
	}

	// Load index
	optionalcorp::Index idx(idxfile);
	
	// Build query
	optionalcorp::BooleanQueryNode qroot(optionalcorp::BooleanQueryNode::OR); // Root
	optionalcorp::QueryNode* p=&qroot;
	for(vector<string>::size_type i = 0; i < terms.size(); ++i)
	{
		p->left(new optionalcorp::TermQueryNode(terms[i],idx));
		if ((i+2)<terms.size())
		{
			p->right(new optionalcorp::BooleanQueryNode(optionalcorp::BooleanQueryNode::OR));
			p=p->right();
		}
		else // last term
		{
			p->right(new optionalcorp::TermQueryNode(terms[i+1],idx));
			break;
		}
	}

	boost::dynamic_bitset<> results(idx.documentCount());
	qroot.getResult(results);
	
	// Output bitmap bit numbers
	size_t hits=0;
	bfs::path docdir("/home/troy/medline/docs/");
	
	for(boost::dynamic_bitset<>::size_type p=results.find_first(); p!=boost::dynamic_bitset<>::npos; p=results.find_next(p))
	{
		if (hits<20)
		{
			// cout << (p+1) << ' ';
			stringstream fname;
			fname << docdir << (p+1) << ".xml";
			ifstream xml(fname.str().c_str());
			string s;
			while (getline(xml,s))
			{
				cout << s;
			}
			cout << endl;
			cout << endl;

		}
		++hits;
	}
	cout << endl;
	
	cout << hits << " hit" << (hits>1?"s":" ") << endl;
	
	return 0;
}