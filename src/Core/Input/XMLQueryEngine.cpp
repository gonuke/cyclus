// XMLQueryEngine.cpp
// Implements class for querying XML snippets

#include "XMLQueryEngine.h"

#include "CycException.h"

//- - - - - - 
XMLQueryEngine::XMLQueryEngine(std::string snippet) {

  char *myEncoding = NULL;
  int myParserOptions = 0;
  doc = xmlReadDoc((const xmlChar*)snippet.c_str(),"",myEncoding,myParserOptions);
  if (NULL == doc) {
    // throw CycParseException("Failed to parse snippet");
  }
  
  xpathCtxt = xmlXPathNewContext(doc);

}

//- - - - - - - 
int XMLQueryEngine::find_elements(const char* expression) {

  int numElements = 0;

  /* Evaluate xpath expression */
  currentXpathObj = xmlXPathEvalExpression((const xmlChar*)expression, xpathCtxt);
  
  if (NULL != currentXpathObj)
    numElements = currentXpathObj->nodesetval->nodeNr;

  return numElements;

}

//- - - - - - 
std::string XMLQueryEngine::get_contents(int elementNum) {

  xmlNodePtr node = currentXpathObj->nodesetval->nodeTab[elementNum];
  std::string XMLcontent;
  xmlBufferPtr nodeBuffer = xmlBufferCreate();

  switch (node->children->type) {
  case XML_ELEMENT_NODE:
    xmlNodeDump(nodeBuffer,doc,node->children,0,1);
    XMLcontent = (const char*)(nodeBuffer->content);
    break;
  case XML_TEXT_NODE:
    XMLcontent = (const char*)(node->children->content);
    break;
  default:
    XMLcontent = "XMLQueryEngine does not currently handle nodes of this type";
  }

  xmlBufferFree(nodeBuffer);

  return XMLcontent;

}

//- - - - - - 
std::string XMLQueryEngine::get_name(int elementNum) {

  std::string XMLname = (const char*)(currentXpathObj->nodesetval->nodeTab[elementNum]->name);

  return XMLname;

}



