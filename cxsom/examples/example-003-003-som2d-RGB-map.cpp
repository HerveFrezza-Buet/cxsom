#include <cxsom-rules.hpp>

// This is for further convenience
using namespace cxsom::rules;

// This is a mandatory declaration, it implements some global variable. 
context* cxsom::rules::ctx = nullptr;


// In this example, we build a 2D SOM, fed with RGB (i.e. 3-sized array) data. 

// This is very similar to example-003-001... see comments there.

#define CACHE       2
#define TRACE   50000
#define OPENED   true
#define FORGET      0

#define WALLTIME TRACE-1

int main(int argc, char* argv[]) {
  context c(argc, argv);

  kwd::parameters p_main;
  p_main                                                                                      | kwd::use("walltime", -1), kwd::use("epsilon", 1e-3);

  {
    timeline t("in");         
    kwd::type("xi", "Array=3", CACHE, FORGET, OPENED); 
    "xi" << fx::random()                                                                      | kwd::use("walltime", WALLTIME);
  }

  {
    timeline t("wgt");
    kwd::type("W", "Map2D<Array=3>=50", CACHE, TRACE, OPENED);
    
    kwd::at("W", 0) << fx::random();
    "W" << fx::learn_gaussian(kwd::var("in", "xi"), kwd::prev("W"), kwd::var("rlx", "BMU"))   | p_main, kwd::use("alpha", .05), kwd::use("sigma", .075);
  }

  {
    timeline t("rlx");  

    kwd::type("A",   "Map2D<Scalar>=50", CACHE, FORGET, OPENED); 
    kwd::type("BMU", "Pos2D",            CACHE, FORGET, OPENED);

    kwd::at("A",   0) << fx::clear()                                                          | kwd::use("value",  0);
    kwd::at("BMU", 0) << fx::clear()                                                          | kwd::use("value", .5);

    "A"   << fx::match_gaussian(kwd::var("in", "xi"), kwd::prev(kwd::var("wgt", "W")))        | p_main, kwd::use("sigma", .1);
    "BMU" << fx::argmax("A")                                                                  | p_main, kwd::use("random-bmu", 1); 
  }

  return 0;
}


