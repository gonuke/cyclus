#ifndef CYCLUS_SRC_DISCOVERY_H_
#define CYCLUS_SRC_DISCOVERY_H_

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <streambuf>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "dynamic_module.h"
#include "env.h"
#include "suffix.h"

namespace cyclus {

/// This function returns a vector of archetype names in a given 
/// string that is the binary represnetation of a module/shared-object/library.
std::set<std::string> DiscoverArchetypes(const std::string s);

/// Discover archetype specifications for a path and library.
std::set<std::string> DiscoverSpecs(std::string p, std::string lib);

/// Discover archetype specifications that live recursively in modules in a dir.
std::set<std::string> DiscoverSpecsInDir(std::string d);

/// Discover archetype specifications that live recursively in CYCLUS_PATH directories.
std::set<std::string> DiscoverSpecsInCyclusPath();

}  // namespace cyclus

#endif  // CYCLUS_SRC_DISCOVERY_H_
