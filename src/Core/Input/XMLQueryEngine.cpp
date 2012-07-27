// XMLQueryEngine.cpp
// Implements class for querying XML snippets

#include "XMLQueryEngine.h"

#include "CycException.h"

//- - - - - - 

XMLQueryEngine::XMLQueryEngine() {
  
  doc = NULL;
  xpathCtxt = NULL;
  numElements = 0;

}

XMLQueryEngine::XMLQueryEngine(std::string snippet) {

  init(snippet);

}

XMLQueryEngine::XMLQueryEngine(xmlDocPtr current_doc) {

  if (NULL == current_doc) {
    throw CycParseException("Invalide xmlDocPtr passed into XMLQueryEnginer");
  }
  
  doc = current_doc;
  xpathCtxt = xmlXPathNewContext(doc);
  numElements = 0;

}

void XMLQueryEngine::init(std::string snippet) {

  char *myEncoding = NULL;
  int myParserOptions = 0;
  doc = xmlReadDoc((const xmlChar*)snippet.c_str(),"",myEncoding,myParserOptions);
  if (NULL == doc) {
    // throw CycParseException("Failed to parse snippet");
  }
  
  xpathCtxt = xmlXPathNewContext(doc);
  numElements = 0;

}


//- - - - - - - 
int XMLQueryEngine::find_elements(const char* expression) {

  numElements = 0;

  /* Evaluate xpath expression */
  currentXpathObj = xmlXPathEvalExpression((const xmlChar*)expression, xpathCtxt);
  
  if (NULL != currentXpathObj)
    numElements = currentXpathObj->nodesetval->nodeNr;

  return numElements;

}

//- - - - - - 
std::string XMLQueryEngine::get_contents(const char* expression) {

  if (0 == find_elements(expression)) {
    throw CycParseException("Can't find an element with the name " + expression);
  }
  
  return get_contents(0);

}
std::string XMLQueryEngine::get_contents(int elementNum) {

  if (elementNum >= numElements) {
    throw CycParseException("Too many elements requested. (" + elementNum + " >= " + numElements + ")");
  }

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

