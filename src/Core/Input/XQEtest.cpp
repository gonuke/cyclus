#include <iostream>

#include "XMLQueryEngine.h"

std::string testXMLcode1 = "<docstart><elementA><subelement><moresub>some content</moresub></subelement></elementA><elementA>data</elementA></docstart>";

boolean test1() {

  XMLQueryEngine* testxml1 = new XMLQueryEngine(testXMLcode1);
  
  delete testxml1;

  return !(NULL == testxml1);

}

boolean test2() {

  XMLQueryEngine* testxml1 = new XMLQueryEngine(testXMLcode1);

  int numNodes = testxml1->find_elements("/docstart/elementA");

  delete testxml1;

  return (numNodes == 2);

}

int main(int argc, char* argv[])
{

  
  
}
