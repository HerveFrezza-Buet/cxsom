#include <cxsom-rules.hpp>

using namespace cxsom::rules;
context* cxsom::rules::ctx = nullptr;

#define MAP_SIZE 500

#define CACHE       2
#define TRACE   10000
#define OPENED   true
#define FORGET      0

#define WALLTIME -1 // Infinite walltime

int main(int argc, char* argv[]) {
  context c(argc, argv);

  kwd::parameters p_main, p_learn, p_match;
  p_main  | kwd::use("walltime", WALLTIME), kwd::use("epsilon", 0);
  p_learn | p_main, kwd::use("alpha", .05), kwd::use("r", .05);
  p_match | p_main, kwd::use("r", .3);
  

  {
    timeline t("wgt");

    kwd::type("We", std::string("Map1D<Scalar>=") + std::to_string(MAP_SIZE), CACHE, TRACE, true);
    kwd::type("Wc", std::string("Map1D<Pos1D>=")  + std::to_string(MAP_SIZE), CACHE, TRACE, true);
    
    kwd::at("We", 0) << fx::random();
    kwd::at("Wc", 0) << fx::random();

    "We" << fx::learn_triangle(kwd::var ("in",  "xi"),            kwd::prev("We"), kwd::var("out", "bmu"))  | p_learn;    
    "Wc" << fx::learn_triangle(kwd::prev(kwd::var("out", "bmu")), kwd::prev("Wc"), kwd::var("out", "bmu"))  | p_learn;    
 
  }

  {
    timeline t("match");

    kwd::type("Ae", kwd::type_of({"wgt", "We"}), CACHE, FORGET, true);
    kwd::type("Ac", kwd::type_of("Ae")         , CACHE, FORGET, true);
    
    kwd::at("Ae", 0) << fx::clear()                                                                         | kwd::use("value", 0);
    kwd::at("Ac", 0) << fx::clear()                                                                         | kwd::use("value", 0);
    "Ae" << fx::match_triangle(kwd::var ("in",   "xi"),           kwd::prev(kwd::var("wgt", "We")))         | p_match;
    "Ac" << fx::match_triangle(kwd::prev(kwd::var("out", "bmu")), kwd::prev(kwd::var("wgt", "Wc")))         | p_match;
  }

  {
    timeline t("merge");

    kwd::type("A", kwd::type_of({"match", "Ae"}), CACHE, FORGET, true);
    
    kwd::at("A", 0) << fx::clear()                                                                          | kwd::use("value", 0);
    "A" << fx::average({kwd::var("match", "Ae"), kwd::var("match", "Ac")})                                  | p_main;
  }

  {
    timeline t("out");
    
    kwd::type("bmu", "Pos1D", CACHE, TRACE, true);
    
    kwd::at("bmu", 0) << fx::random();
    "bmu" << fx::conv_argmax(kwd::var("merge", "A"))                                                        | p_main, kwd::use("random-bmu", 1), kwd::use("sigma", .0125);
  }

  {
    timeline t("in");
    kwd::type("xi", "Scalar", CACHE, TRACE, true);
    kwd::at("xi", 0) << fx::random(); // At least, we create the first instance of the variable if necessary.
  }
  
  return 0;
}
