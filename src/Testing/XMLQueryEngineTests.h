// XMLQueryEngineTests.h
#include <gtest/gtest.h>

#include <string>

#include "XMLQueryEngine.h"

//- - - - - - - - - - - - - - - - - 
class XQETest : public ::testing::Test {

 protected:

  XMLQueryEngine *xqe;

  struct TestElement {
    std::string name;
    std::string content;
    std::string element;

    TestElement(std::string nm, std::string cntnt) { 
      name=nm; 
      content=cntnt; 
      element="<" + name ">" + content + "</" + name + ">";
    }
  } testElementA, testElementB, testElementAC;

  struct TestSnippet {
    std::string snippet;
    int numA, numB;
    
  } testSnippetA, testSnippetB, testSnippetC, testSnippetD, 
    testSnippetE, testSnippetF, testSnippetG, testSnippetH;

  XMLQueryEngine *xqe;
  XMLQueryEngine xqeA;

  virtual void SetUp() {
    
    testElementA = TestElement("elementA","contentA");
    testElementB = TestElement("elementB","contentB");
    testElementAC = TestElement("elementA","contentC");

    testSnippetA.snippet = "<start>" + testElementA.element + "</start>";
    testSnippetA.numA = 1;
    testSnippetB.numB = 0;

    testSnippetB.snippet = "<start>" + testElementA.element + testElementB.element + "</start>";
    testSnippetB.numA = 1;
    testSnippetB.numB = 1;

    testSnippetC.snippet = "<start>" + testElementB.element + testElementA.element + "</start>";
    testSnippetC.numA = 1;
    testSnippetC.numB = 1;

    testSnippetD.snippet = "<start>" + testElementA.element + testElementA.element + "</start>";
    testSnippetD.numA = 2;
    testSnippetD.numB = 0;

    testSnippetE.snippet = "<start>" + testElementA.element + testElementA.element + 
                                     + testElementB.element + "</start>";
    testSnippetE.numA = 2;
    testSnippetE.numB = 1;

    testSnippetF.snippet = "<start>" + testElementA.element + testElementA.element + 
                                     + testElementB.element + testElementB.element + "</start>";
    testSnippetF.numA = 2;
    testSnippetF.numB = 2;

    testSnippetG.snippet = "<start>" + testElementA.element + testElementB.element + 
                                     + testElementA.element + testElementB.element + "</start>";
    testSnippetG.numA = 2;
    testSnippetG.numB = 2;

    testSnippetH.snippet = "<start>" + testElementA.element + testElementB.element + 
                                     + testElementAC.element + testElementB.element + "</start>";
    testSnippetH.numA = 2;
    testSnippetH.numB = 2;


  }

  virtual void TearDown() {

  }

}
