#include <cxsom-rules.hpp>

// This is basic cxsom code, we do not use cxsom-builder. At each
// time, the user provides a scalar input (in [0, 1]), and the system
// chooses two other numbers randomly. The output is the min and the
// max of these three numbers.
 
// This is for further convenience
using namespace cxsom::rules;
 
// This is a mandatory declaration, it implements some global variable. 
context* cxsom::rules::ctx = nullptr;
 
// This example introduces the simulation rule definitions.
 
#define FOREVER        -1 // This ends the pattern expansion. -1 means "never ending expansion".
#define CACHE_SIZE      2 // This is the cache size at simulator/processor side
#define BUF_SIZE     1000 // This is the file circular buffer size.
#define KEPT_OPENED  true // Keeping the file opened saves time but
                          // there is a system limit of the number of
                          // files simultaneously opened at simulator
                          // side.

 
int main(int argc, char* argv[]) {
  context c(argc, argv);
  
  {
    timeline t("in");
    kwd::type("X", "Scalar", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
  }
  
  {
    timeline t("internals");
    kwd::type("A", "Scalar", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    kwd::type("B", "Scalar", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    kwd::type("C", "Scalar", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    kwd::type("D", "Scalar", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    kwd::type("E", "Scalar", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);

    // We set internals to a random value when the user input is ready.
    "A" << fx::random_when({kwd::var("in", "X")}) |  kwd::use("walltime", FOREVER);
    "B" << fx::random_when({kwd::var("in", "X")}) |  kwd::use("walltime", FOREVER);
    "C" << fx::random_when({kwd::var("in", "X")}) |  kwd::use("walltime", FOREVER);
    "D" << fx::random_when({kwd::var("in", "X")}) |  kwd::use("walltime", FOREVER);
    "E" << fx::random_when({kwd::var("in", "X")}) |  kwd::use("walltime", FOREVER);

  }
  
  {
    timeline t("out");
    kwd::type("min", "Scalar", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    kwd::type("max", "Scalar", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    
    "min" << fx::min({
	kwd::var("in",        "X"),
	kwd::var("internals", "A"),
	kwd::var("internals", "B"),
	kwd::var("internals", "C"),
	kwd::var("internals", "D"),
	kwd::var("internals", "E")
      })                               |  kwd::use("walltime", FOREVER);
    
    "max" << fx::max({
	kwd::var("in", "X"),
	kwd::var("internals", "A"),
	kwd::var("internals", "B"),
	kwd::var("internals", "C"),
	kwd::var("internals", "D"),
	kwd::var("internals", "E")
      })                               |  kwd::use("walltime", FOREVER);
  }
  
 
  return 0;
}
