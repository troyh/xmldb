#include <xqilla/xqilla-simple.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <boost/filesystem.hpp>

class XQueryResult
{
	Result m_result;
	DynamicContext* m_context;
public:
	XQueryResult(const Result& result,DynamicContext* context);
	~XQueryResult();
	Item::Ptr next();
	inline DynamicContext* getContext() { return m_context; }
};

class XMLDoc
{
    // Initialise Xerces-C and XQilla by creating the factory object
	XQilla m_xqilla;
	xercesc::InputSource* m_inputsource;
public:	
	XMLDoc(const boost::filesystem::path& xmlfile);
	XMLDoc(xercesc::InputSource* input);
	~XMLDoc();
	
	XQueryResult xquery(std::string& querystr);
	XQueryResult xquery(const XMLCh* querystr);

};

