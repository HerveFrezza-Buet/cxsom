#include <cxsom-rules.hpp>

// This is for further convenience
using namespace cxsom::rules;

// This is a mandatory declaration, it implements some global variable. 
context* cxsom::rules::ctx = nullptr;


// In this example, we build a 1D SOM, fed with 2D data.

// This is very similar to example-003-001... see comments there.

#define CACHE       2
#define TRACE   50000
#define OPENED   true
#define FORGET      0

#define WALLTIME TRACE-1

int main(int argc, char* argv[]) {
  context c(argc, argv);

  kwd::parameters p_main;
  p_main                                                                                       | kwd::use("walltime", WALLTIME), kwd::use("epsilon", 1e-3);

  {
    timeline t("in");         
    kwd::type("xi", "Pos2D", CACHE, FORGET, OPENED);

    "xi" << fx::random() | kwd::use("walltime", WALLTIME);
  }

  {
    timeline t("wgt");   
    kwd::type("W", "Map1D<Pos2D>=500", CACHE, TRACE, OPENED);

    kwd::at("W", 0) << fx::random();
    "W" << fx::learn_gaussian(kwd::var("in", "xi"), kwd::prev("W"), kwd::var("rlx", "BMU"))    | p_main, kwd::use("alpha", .1), kwd::use("sigma", .01);
  }

  {
    timeline t("rlx");  
    kwd::type("A",    "Map1D<Scalar>=500", CACHE, FORGET, OPENED); 
    kwd::type("BMU", "Pos1D",              CACHE, FORGET, OPENED);           

    kwd::at("A",   0) << fx::clear()                                                           | kwd::use("value",  0);
    kwd::at("BMU", 0) << fx::clear()                                                           | kwd::use("value", .5); 

    "A"   << fx::match_gaussian(kwd::var("in", "xi"), kwd::prev(kwd::var("wgt", "W")))         | p_main, kwd::use("sigma", .1);
    "BMU" << fx::argmax("A")                                                                   | p_main, kwd::use("random-bmu", 1);
  }

  return 0;
}

