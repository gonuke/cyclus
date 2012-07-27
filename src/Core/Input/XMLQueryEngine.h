//XMLQueryEngine.h
#if !defined(_XMLQUERYENGINE_H)
#define _XMLQUERYENGINE_H

#include <string>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/relaxng.h>

/**
   A class for extracting information from XML snippets

   This class provides a simple mechanism for retreiving information
   from an XML snippet using the XPath query infrastructure.

**/

class XMLQueryEngine {

 private:
  int numElements;

  xmlDocPtr doc;

  xmlXPathContextPtr xpathCtxt;

  xmlXPathObjectPtr currentXpathObj;

  void init(std::string expression);

 public:
  XMLQueryEngine(std::string snippet);
  XMLQueryEngine(xmlDocPtr current_doc);
  
  int find_elements(const char* expression);
  
  std::string get_contents(const char* expression);
  std::string get_contents(int elementNum=0);
  
  std::string get_name(int elementNum);

};

#endif
