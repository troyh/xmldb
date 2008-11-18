#ifndef _OUZO_QUERYTREE_HPP
#define _OUZO_QUERYTREE_HPP

#include "Index.hpp"
#include "DocSet.hpp"


namespace Ouzo
{
	class DocumentBase;
	
	namespace Query
	{
		class Node
		{
			string m_dbname;
		protected:
			Node* m_parent;
			Node* m_left;
			Node* m_right;
		public:
			Node(const string& dbname) : m_dbname(dbname), m_parent(0), m_left(0), m_right(0) {}
			virtual ~Node() { delete m_left; delete m_right; }
	
			virtual void left(Node* p) {m_left=p;if (p) p->m_parent=this;}
			virtual Node* left() const {return m_left;}
	
			virtual void right(Node* p){m_right=p;if (p) p->m_parent=this;}
			virtual Node* right() const {return m_right;}
	
			virtual void parent(Node* p) { m_parent=p; }
			virtual Node* parent() const { return m_parent; }
	
			virtual bool isTermNode() const=0;
			virtual bool isOperNode() const=0;
	
			// virtual size_t countOfTermNodes() const;
			// virtual void getResult(boost::dynamic_bitset<>& result)=0;

			const string& getDocBaseName() const { return m_dbname; }
		};
		
		ostream& operator<<(ostream& os, Node&);

		class BooleanNode: public Node
		{
		public:
			typedef enum { OR, AND, UNDEF } boolean_type;
		protected:
			boolean_type m_type;
			bool m_unarynot;
		public:
			BooleanNode(const string& dbname, boolean_type type=UNDEF) : Node(dbname), m_type(type), m_unarynot(false) {}
	
			bool isTermNode() const { return false; }
			bool isOperNode() const { return true; }

			void getResult(boost::dynamic_bitset<>& result);
	
			void oper(boolean_type type) { m_type=type; }
			boolean_type oper() const { return m_type; }
			
			bool isUnaryNot() const { return m_unarynot; }
		};

		class TermNode : public Node
		{
		public:
			typedef enum {eq,ne,lt,lte,gt,gte} equality_op;
		private:
			string m_idxname;
			equality_op m_eqop;
			Index::key_t m_val;
		public:
			TermNode(string& dbname,string& indexname,equality_op eqop, Index::key_t& val) 
				: Node(dbname), m_idxname(indexname), m_eqop(eqop), m_val(val) {}
	
			bool isTermNode() const { return true; }
			bool isOperNode() const { return false; }

			void getResult(boost::dynamic_bitset<>& result);
	
			const string& indexname() const { return m_idxname; }
			equality_op eqop() const { return m_eqop; }
			const Index::key_t& val() const { return m_val; }
		
		};
	
		ostream& operator<<(ostream& os, TermNode::equality_op op);

		class Results : public DocSet
		{
			DocumentBase* m_pDB;
			double m_timer;
		public:
			Results(DocumentBase* pDB);
			Results(const Results& r);
			~Results() {}
			
			Results& operator=(const DocSet& ds);
			Results& operator=(const Results& r);
			
			DocumentBase* docbase() const { return m_pDB; }
			
			double queryTime() const { return m_timer; }
			void queryTime(double n) { m_timer=n; }
		};
	}
}

#endif
