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
#include <boost/spirit/core.hpp>
#include <boost/spirit/actor/insert_at_actor.hpp>


using namespace std;
using namespace boost::spirit;
namespace bfs=boost::filesystem;


namespace optionalcorp
{
	class Index
	{
		typedef map< string, vector<uint32_t> > index_type;
		
		index_type m_idx;
		bfs::path m_filename;
		
	public:
		Index(bfs::path index_file);
		
		inline vector<uint32_t>& operator[](string key) 
		{
			return m_idx[key];
		}
		
		size_t documentCount() const;
		
		const bfs::path& filename() const { return m_filename; }
		
		void load();
	};
	
	
	class QueryNode
	{
	protected:
		QueryNode* m_parent;
		QueryNode* m_left;
		QueryNode* m_right;
	public:
		QueryNode() : m_parent(0), m_left(0), m_right(0) {}
		virtual ~QueryNode() { delete m_left; delete m_right; }
		
		virtual void left(QueryNode* p){m_left=p;if (p) p->m_parent=this;}
		virtual QueryNode* left(){return m_left;}
		
		virtual void right(QueryNode* p){m_right=p;if (p) p->m_parent=this;}
		virtual QueryNode* right(){return m_right;}
		
		virtual void parent(QueryNode* p) { m_parent=p; }
		virtual QueryNode* parent() { return m_parent; }
		
		virtual bool isTermNode() const=0;
		virtual bool isOperNode() const=0;
		
		virtual size_t countOfTermNodes() const;
		virtual void getResult(boost::dynamic_bitset<>& result)=0;
		
		virtual size_t documentCount() const;
	};
	
	class BooleanQueryNode: public QueryNode
	{
	public:
		typedef enum { OR, AND, NOT, UNDEF } boolean_type;
	protected:
		boolean_type m_type;
	public:
		BooleanQueryNode(boolean_type type=UNDEF) : m_type(type) {}
		
		bool isTermNode() const { return false; }
		bool isOperNode() const { return true; }

		void getResult(boost::dynamic_bitset<>& result);
		
		void oper(boolean_type type) { m_type=type; }
		boolean_type oper() const { return m_type; }
	};
	
	class TermQueryNode : public QueryNode
	{
		string m_term;
		Index m_idx;
	public:
		TermQueryNode(string& str,string& key) : m_term(str), m_idx(key) {}
		
		bool isTermNode() const { return true; }
		bool isOperNode() const { return false; }

		void getResult(boost::dynamic_bitset<>& result);
		
		const string& term() const { return m_term; }
		const Index& index() const { return m_idx; }

		size_t documentCount() const;
	};
	
	ostream& operator<<(ostream& os, QueryNode*);

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
	
	size_t QueryNode::documentCount() const
	{
		size_t left=0,right=0;
		if (m_left)
			left=m_left->documentCount();
		if (m_right)
			right=m_right->documentCount();
		return max(left,right);
	}
	
	void TermQueryNode::getResult(boost::dynamic_bitset<>& result)
	{
		m_idx.load();
		
		result.reset();
		
		for(size_t j = 0, jj=m_idx[m_term].size(); j < jj; ++j)
		{
			boost::dynamic_bitset<>::size_type bitno=(m_idx[m_term])[j]-1;
			result.set(bitno);
		}
		
	}
	
	size_t TermQueryNode::documentCount() const
	{
		return m_idx.documentCount();
	}
	

	void BooleanQueryNode::getResult(boost::dynamic_bitset<>& result)
	{
		// we have 2 children, get query results from both children
		boost::dynamic_bitset<> leftresult(result.size());
		if (m_left)
			m_left->getResult(leftresult);
		
		boost::dynamic_bitset<> rightresult(result.size());
		if (m_right)
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

	Index::Index(bfs::path index_file)
		: m_filename(change_extension(index_file,".index"))
	{
	}

	void Index::load() {
		// TODO: Don't load the same index multiple times!
		cout << "Loading index:" << m_filename << endl;
		std::ifstream ifs(m_filename.string().c_str());
		boost::archive::text_iarchive ar(ifs);
		boost::serialization::load(ar,m_idx,0);
	}
	
	size_t Index::documentCount() const
	{
		return 30000; // TODO: figure out actual value
	}

	ostream& operator<<(ostream& os, QueryNode* node)
	{
		if (node->left())
		{
			os << '(';
			os << node->left();
			os << ')';
		}
		
		if (node->isTermNode())
		{
			os << dynamic_cast<optionalcorp::TermQueryNode*>(node)->index().filename() 
				<< ':';
			os << dynamic_cast<optionalcorp::TermQueryNode*>(node)->term();
		}
		else
		{
			switch (dynamic_cast<optionalcorp::BooleanQueryNode*>(node)->oper())
			{
			case optionalcorp::BooleanQueryNode::OR:
				os << '|';
				break;
			case optionalcorp::BooleanQueryNode::AND:
				os << '&';
				break;
			case optionalcorp::BooleanQueryNode::NOT:
				os << '^';
				break;
			}
		}
		
		if (node->right())
		{
			os << '(';
			os << node->right();
			os << ')';
		}
		
		return os;
	}
	
}

namespace {
	string g_key;
	optionalcorp::BooleanQueryNode query_root;
	optionalcorp::QueryNode* query_iter=&query_root;
	size_t g_groupdepth=0;
	
void do_key(char const* str, char const* end)
{
	string s(str,end);
	// cout << "key:" << s << endl;
	g_key=s;
}
void do_val(char const* str, char const* end)
{
	string s(str,end);
	// cout << g_key << "=" << s << endl;
	optionalcorp::TermQueryNode* pNode=new optionalcorp::TermQueryNode(s,g_key);
	if (!query_iter->left())
		query_iter->left(pNode);
	else
		query_iter->right(pNode);
	query_iter=pNode;
}

void do_bool(char c)
{
	if (!query_iter->parent()) // At root already
	{
		// Push the entire tree down a level
		optionalcorp::BooleanQueryNode* pNewNode=new optionalcorp::BooleanQueryNode;
		pNewNode->oper(((optionalcorp::BooleanQueryNode*)query_iter)->oper());
		pNewNode->left(query_iter->left());
		pNewNode->right(query_iter->right());
		
		query_iter->left(pNewNode);
		query_iter->right(0);
	}
	else
	{
		query_iter=query_iter->parent();
		
		if (!query_iter->isOperNode())
			throw new exception();
		
		if (query_iter->left() && query_iter->right())
		{ // This node is full, add a new node on the right side
			optionalcorp::BooleanQueryNode* pNewNode=new optionalcorp::BooleanQueryNode;
			pNewNode->left(query_iter->right());
			query_iter->right(pNewNode);
			query_iter=pNewNode;
		}
	}
	
	switch (c)
	{
	case '|':
		// cout << "OR" << endl;
		((optionalcorp::BooleanQueryNode*)query_iter)->oper(optionalcorp::BooleanQueryNode::OR);
		break;
	case '&':
		// cout << "AND" << endl;
		((optionalcorp::BooleanQueryNode*)query_iter)->oper(optionalcorp::BooleanQueryNode::AND);
		break;
	case '^':
		// cout << "NOT" << endl;
		((optionalcorp::BooleanQueryNode*)query_iter)->oper(optionalcorp::BooleanQueryNode::NOT);
		break;
	default:
		break;
	}

	++g_groupdepth;
}

void do_group(char const* str,char const* end)
{
	// cout << "()" << endl;
	while (g_groupdepth--)
	{
		if (query_iter->parent())
			query_iter=query_iter->parent();
	}
}

}

struct querygrammar : public grammar<querygrammar>
{
	template <typename ScannerT>
	struct definition
	{
		definition(querygrammar const&)
		{
			query = clause_group >>  *( boolop >> clause_group );
			clause_group = '(' >> (clause >> *( boolop >> clause ))[&do_group] >> ')'
						 | (clause >> *( boolop >> clause ))[&do_group];
			clause  = (key[&do_key] >> '=' >> val[&do_val]);
			boolop = ch_p('|')[&do_bool] | ch_p('&')[&do_bool] | ch_p('^')[&do_bool];
			key = alpha_p >> *alpha_p;
			val = alpha_p >> *alpha_p;
		}
		
		rule<ScannerT> query,clause_group,clause,boolop,key,val;
		rule<ScannerT> const& start() const { return query; }
	};
};


int main(int argc,char* argv[])
{
	boost::progress_timer query_timer;

	// bfs::path idxfile(argv[1]);

	vector<string> terms;
	for (int i=2;i<argc;++i)
	{
		terms.push_back(argv[i]);
	}
	
	querygrammar query;
	parse_info<> info=parse(argv[1],query,space_p);
	if (info.full)
	{
		cout << "Query:" << &query_root << endl;
	}
	else
	{
		cout << "Failed parse:" << info.stop << endl;
		return -1;
	}

	boost::dynamic_bitset<> results(query_root.documentCount());
	query_root.getResult(results);
	
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
	
	cout << hits << " hit" << (hits==1?"":"s") << endl;
	
	return 0;
}