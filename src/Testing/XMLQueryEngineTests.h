// XMLQueryEngineTests.h
#include <gtest/gtest.h>
#include "XMLQueryEngine.h"

//- - - - - - - - - - - - - - - - - 
class XQETest : public ::testing::Test {

 protected:

  XMLQueryEngine* empty_xqe;
  XMLQueryEngine* simple_xqe;


  vritual void SetUp() {
    empty_xqe = new XMLQueryEngine();
    simple_xqe = new XMLQueryEngine("<simulation><elementA>content for elementA</elementA></simulation>")

  }

  virtual void TearDown() {

  }

}
