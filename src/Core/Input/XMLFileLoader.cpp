// XMLFileLoader.cpp
// Implements file reader for an XML format
#include <iostream>
#include <string>
#include <sys/stat.h>

#include "XMLFileLoader.h"

#include "Timer.h"
#include "RecipeLibrary.h"
#include "Model.h"

#include "Env.h"
#include "CycException.h"
#include "Logger.h"

using namespace std;

XMLFileLoader::XMLFileLoader(std::string load_filename) {
  // double check that the file exists
  if("" == load_filename) {
    throw CycIOException("No input filename was given");
  } else { 
    FILE* file = fopen(load_filename.c_str(),"r");
    if (NULL == file) { 
      throw CycIOException("The file '" + filename
           + "' cannot be loaded because it has not been found.");
    }
    fclose(file);
  }

  filename = load_filename;
}


void XMLFileLoader::validate_file(std::string schema_file) {

  xmlRelaxNGParserCtxtPtr ctxt = xmlRelaxNGNewParserCtxt(schema_file.c_str());
  if (NULL == ctxt)
    throw CycParseException("Failed to generate parser from schema: " + schema_file);

  xmlRelaxNGPtr schema = xmlRelaxNGParse(ctxt);

  xmlRelaxNGValidCtxtPtr vctxt = xmlRelaxNGNewValidCtxt(schema);

  doc = xmlReadFile(filename.c_str(), NULL,0);
  if (NULL == doc) {
    throw CycParseException("Failed to parse: " + filename);
  }

  if (xmlRelaxNGValidateDoc(vctxt,doc))
    throw CycParseException("Invalid XML file; file: "    
      + filename 
      + " does not validate against schema " 
      + schema_file);

  /// free up some data ???

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void XMLFileLoader::load_catalog(std::string catalogElement, CatalogType catalogType, 
				 std::string cur_ns) {

  XMLQueryEngine xqe(catalogElement);

  string cat_filename, ns, format;
  cat_filename = xqe.get_contents("filename");
  ns = xqe.get_contents("namespace");
  format = xqe.get_contents("format");
  
  if ("xml" == format) {
    CLOG(LEV_DEBUG2) << "going into a catalog...";
    XMLFileLoader catalog(cat_filename);
    catalog.validate_file();
    if (recipeBook == catalogType) {
      catalog.load_recipes(cur_ns + ns + ":");
    } else if (facilityCatalog == catalogType) {
      catalog.load_facilities(cur_ns + ns + ":");
    }
  } 
  else {
    throw 
      CycRangeException(format + "is not a supported catalog format.");
  }

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void XMLFileLoader::load_recipes(std::string cur_ns) {

  XMLQueryEngine xqe(doc);

  int numRecipeBooks = xqe.find_elements("/*/recipebook");
  for (int rb_num=0;rb_num<numRecipeBooks;rb_num++) {
    load_catalog("recipeBook",xqe.get_contents(rb_num),cur_ns);
  }

  int numRecipes = xqe.find_elements("/*/recipe");
  for (int recipe_num=0;recipe_num<numRecipes;recipe_num++) {
    RecipeLibrary::load_recipe(xqe.get_contents(recipe_num),cur_ns);
  }

}

void XMLFileLoader::load_facilities(std::string cur_ns) {

  XMLQueryEngine xqe(doc);

  int numFacCats = xqe.find_elements("/*/facilitycatalog");
  for (int fac_cat_num=0;fac_cat_num<numFacCats;fac_cat_num++) {
    load_catalog("facilityCatalog",xqe.get_contents(fac_cat_num),cur_ns);
  }

  load_models("/*/facility","Facility");

}


void XMLFileLoader::load_all_models() {

  load_models("/*/converter","Converter");
  load_models("/*/market","Market");
  load_facilities("");
  load_models("/simulation/region","Region");
  load_models("/simulation/region/institution","Inst")
}

void XMLFileLoader::load_models(std::string modelPath, std::string factoryType) {

  XMLQueryEngine xqe(doc);

  int numModels = xqe.find_elements(modelPath);
  for (int model_num=0;model_num<numModels;model_num++)
    ModelFactory::create(factoryType,xqe.get_contents(model_num));
  
}

void XMLFileLoader::load_params() {

  XMLQueryEngine xqe(doc);
  
  TI->initialize(xqe.get_contents("/control"));
  
}
  
