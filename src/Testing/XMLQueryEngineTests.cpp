// XMLQueryEngineTests.cpp
#include <getst/gtest.h>
#include <string>
#include "XMLQueryEngineTests.h"



//- - - - - - - - - - - - - - 
TEST_F(XQETest, Constructors) {

  ASSERT_NO_THROW(xqe = new XMLQueryEngine());
  EXPECT_NE(xqe, NULL);
  ASSERT_NO_THROW(delete xqe);

  ASSERT_NO_THROW(xqe = new XMLQueryEngine(testSnippetA));
  EXPECT_NE(xqe, NULL);
  ASSERT_NO_THORW(delete xqe);

  // need to add test for the construtor based on an existing doc
}

TEST_F(XQETest, SearchSingle) {

  ASSERT_NO_THROW(xqeA = XMLQueryEngine(testSnippetA.snippet));
  EXPECT_EQ(xqeA.find_element(testElementA.name),testSnippetA.numA);
  EXPECT_EQ(xqeA.find_element(testElementB.name),testSnippetA.numB);

}

TEST_F(XQETest, SearchMulti) {

  ASSERT_NO_THROW(xqeA = XMLQuertEngine(testSnippetB.snippet));
  EXPECT_EQ(xqeA.find_element(testElementA.name),testSnippetB.numA);
  EXPECT_EQ(xqeA.find_element(testElementB.name),testSnippetB.numB);

  ASSERT_NO_THROW(xqeA = XMLQuertEngine(testSnippetC.snippet));
  EXPECT_EQ(xqeA.find_element(testElementA.name),testSnippetC.numA);
  EXPECT_EQ(xqeA.find_element(testElementB.name),testSnippetC.numB);

}

TEST_F(XQETest, SearchCombi) {

  ASSERT_NO_THROW(xqeA = XMLQueryEngine(testSnippetC.snippet));

  EXPECT_EQ(xqeA.find_element(testElementA.name),testSnippetC.numA);
  EXPECT_EQ(xqeA.find_element(testElementB.name),testSnippetC.numB);

  ASSERT_NO_THROW(xqeA = XMLQueryEngine(testSnippetD.snippet));

  EXPECT_EQ(xqeA.find_element(testElementA.name),testSnippetD.numA);
  EXPECT_EQ(xqeA.find_element(testElementB.name),testSnippetD.numB);

  ASSERT_NO_THROW(xqeA = XMLQueryEngine(testSnippetE.snippet));

  EXPECT_EQ(xqeA.find_element(testElementA.name),testSnippetE.numA);
  EXPECT_EQ(xqeA.find_element(testElementB.name),testSnippetE.numB);

  ASSERT_NO_THROW(xqeA = XMLQueryEngine(testSnippetF.snippet));

  EXPECT_EQ(xqeA.find_element(testElementA.name),testSnippetF.numA);
  EXPECT_EQ(xqeA.find_element(testElementB.name),testSnippetF.numB);

  ASSERT_NO_THROW(xqeA = XMLQueryEngine(testSnippetG.snippet));

  EXPECT_EQ(xqeA.find_element(testElementA.name),testSnippetG.numA);
  EXPECT_EQ(xqeA.find_element(testElementB.name),testSnippetG.numB);

  ASSERT_NO_THROW(xqeA = XMLQueryEngine(testSnippetH.snippet));

  EXPECT_EQ(xqeA.find_element(testElementA.name),testSnippetH.numA);
  EXPECT_EQ(xqeA.find_element(testElementB.name),testSnippetH.numB);
  ASSERT_NO_THROW(delete xqe);

}

TEST_F(XQETest, ExtractSingleByName) {

  ASSERT_NO_THROW(xqeA = XMLQuertEngine(testSnippetA.snippet));

  EXPECT_EQ(xqeA.get_contents(testElementA.name),testElementA.content);


}

TEST_F(XQETest, ExtractSingleByCount) {

  ASSERT_NO_THROW(xqeA = XMLQuertEngine(testSnippetA.snippet));

  EXPECT_EQ(xqeA.find_element(testElementA.name),testSnippetA.numA);
  EXPECT_EQ(xqeA.get_contents(0),testElementA.content);
  EXPECT_EQ(xqeA.get_contents(),testElementA.content);

}
  
TEST_F(XQETest, ExtractMulti) {

  int numA, elenum; 

  ASSERT_NO_THROW(xqeA = XMLQuertEngine(testSnippetD.snippet));
  EXPECT_EQ(numA = xqeA.find_element(testElementA.name),testSnippetD.numA);
  for (elenum=0;elenum<numA;elenum++) {
    EXPECT_EQ(xqeA.get_contents(0),testElementA.content);
  }

  ASSERT_NO_THROW(xqeA = XMLQuertEngine(testSnippetE.snippet));
  EXPECT_EQ(numA = xqeA.find_element(testElementA.name),testSnippetE.numA);
  for (elenum=0;elenum<numA;elenum++) {
    EXPECT_EQ(xqeA.get_contents(0),testElementA.content);
  }

  ASSERT_NO_THROW(xqeA = XMLQuertEngine(testSnippetF.snippet));
  EXPECT_EQ(numA = xqeA.find_element(testElementA.name),testSnippetF.numA);
  for (elenum=0;elenum<numA;elenum++) {
    EXPECT_EQ(xqeA.get_contents(0),testElementA.content);
  }

  ASSERT_NO_THROW(xqeA = XMLQuertEngine(testSnippetG.snippet));
  EXPECT_EQ(numA = xqeA.find_element(testElementA.name),testSnippetG.numA);
  for (elenum=0;elenum<numA;elenum++) {
    EXPECT_EQ(xqeA.get_contents(0),testElementA.content);
  }

  ASSERT_NO_THROW(xqeA = XMLQuertEngine(testSnippetH.snippet));
  EXPECT_EQ(numA = xqeA.find_element(testElementA.name),testSnippetH.numA);
  for (elenum=0;elenum<numA;elenum++) {
    EXPECT_EQ(xqeA.get_contents(0),testElementA.content);
  }

}
