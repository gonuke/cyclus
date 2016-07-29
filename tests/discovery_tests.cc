#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "discovery.h"
#include "env.h"

TEST(DiscoveryTests, DiscoverArchetypes) {
  using std::string;
  using std::set;
  string s = "RS4_^@_Construct<std::pair<std::basic_string<char>, std::basic_"
             "string<char> >, std::pair<std::basic_string<char>, std::basic_s"
             "tring<char> > >^@_ZNSt6vectorIS_IN5boost6spirit14basic_hold_any"
             "IcEESaIS3_EESaIS5_EE5beginEv^@_ZNSt8_Rb_treeIN6cyclus18Capacity"
             "ConstraintINS0_8MaterialEEES3_St9_IdentityIS3_ESt4lessIS3_ESaIS"
             "3_EE13_Rb_tree_implIS7_Lb0EE13_M_initializeEv^@_ZNKSt3setIN5boo"
             "st10shared_ptrIN6cyclus12BidPortfolioINS2_8MaterialEEEEESt4less"
             "IS6_ESaIS6_EE11lower_boundERKS6_^@_ZSt10_ConstructISsSsEvPT_RKT"
             "0_^@allocator<std::_Rb_tree_node<std::pair<int const, std::vect"
             "ConstructMyFacility-or<std::pair<std::basic_string<char>, doubl"
             "e>ConstructYourself> > > >^@_internal_equiv^@_ZN6cyclus10IsNega"
             "tiveEd^@_ZNSt8_Rb_treeISsSt4pairIKSsN5boost10shared_ptrIN6cyclu"
             "e>_ConstructYourself> > > >^@_internal_equiv^@_ZN6cyclus1IsNega"
             "s8MaterialEEEESt10_Select1stIS7_~Construct_ME_42~ESt4lessISsESa"
             "s8MaerialEEEESt10_Select1stIS7_~_Construct_ME_42~ESt4lessISsESa"
             "IS7_EE8_S_valueEPKSt18_Rb_tree_node_base^@_ZNSt8_Rb_treeIN5boos"
             "t10shared_ptrIN6cyclus12BidPortfolioINS2_8MaterialEEEEES6_St9_I"
             "dentityIS6_ESt4lessIS6_ESaIS6_EE7_M_rootEv^@_ZNK5boost6spirit14"
             "basic_hold_anyIcE4castISsEERKT_v^@_ZNSt6vectorIN6cyclus7DbTypes";
  set<string> obs = cyclus::DiscoverArchetypes(s);
  set<string> exp;
  exp.insert("Yourself");
  exp.insert("_ME_42");
  EXPECT_EQ(exp, obs);
}

TEST(DiscoveryTests, DiscoverSpec) {
  using std::string;
  using std::set;
  set<string> obs = cyclus::DiscoverSpecs("", "agents");
  set<string> exp;
  exp.insert(":agents:NullInst");
  exp.insert(":agents:NullRegion");
  exp.insert(":agents:Sink");
  exp.insert(":agents:Source");
  exp.insert(":agents:KFacility");
  exp.insert(":agents:Prey");
  exp.insert(":agents:Predator");
  EXPECT_EQ(exp, obs);
}

TEST(DiscoveryTests, DiscoverSpecsInInstallPath) {
  using std::string;
  using std::set;
  set<string> obs = cyclus::DiscoverSpecsInDir(cyclus::Env::GetInstallPath() + \
                                               "/lib/cyclus");
  set<string> exp;
  exp.insert(":agents:NullInst");
  exp.insert(":agents:NullRegion");
  exp.insert(":agents:Sink");
  exp.insert(":agents:Source");
  exp.insert(":agents:KFacility");
  exp.insert(":agents:Prey");
  exp.insert(":agents:Predator");
  for (set<string>::iterator it = exp.begin(); it != exp.end(); ++it)
    EXPECT_EQ(1, obs.count(*it));
}
