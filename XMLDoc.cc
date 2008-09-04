#include <boost/filesystem.hpp>
#include "XMLDoc.hpp"

XMLDoc::XMLDoc(const boost::filesystem::path& xmlfile)
	: m_inputsource(0)
{
	m_inputsource=new xercesc::LocalFileInputSource(X(xmlfile.string().c_str()));
}

XMLDoc::XMLDoc(xercesc::InputSource* input)
	: m_inputsource(input)
{
}

XMLDoc::~XMLDoc()
{
}

XQueryResult XMLDoc::xquery(std::string& querystr)
{
    return this->xquery(X(querystr.c_str()));
}

XQueryResult XMLDoc::xquery(const XMLCh* querystr)
{
    // Parse an XQuery expression
    XQQuery* query=m_xqilla.parse(querystr);

    // Create a context object
    DynamicContext* context=query->createDynamicContext();

	// Parse a document, and set it as the context item
	// Sequence seq = context->resolveDocument(X(m_xmlfile.string().c_str()), 0);
	Sequence seq(context->parseDocument(*m_inputsource));
	
	if(!seq.isEmpty() && seq.first()->isNode()) 
	{
	       context->setContextItem(seq.first());
	       context->setContextPosition(1);
	       context->setContextSize(1);
	}

	// Execute the query, using the context
	XQueryResult result(query->execute(context),context);

	return result;
}

XQueryResult::XQueryResult(const Result& result, DynamicContext* context)
	: m_result(result), m_context(context)
{
}

XQueryResult::~XQueryResult()
{
	delete m_context;
}

Item::Ptr XQueryResult::next()
{
	return m_result->next(m_context);
}
