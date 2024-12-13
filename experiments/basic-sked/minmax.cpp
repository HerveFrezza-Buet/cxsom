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
 
#define WALLTIME       -1 // This ends the pattern expansion. -1 means "never ending expansion".
#define CACHE_SIZE      2 // This is the cache size at simulator/processor side
#define BUF_SIZE     1000 // This is the file circular buffer size.
#define KEPT_OPENED  true // Keeping the file opened saves time but
                          // there is a system limit of the number of
                          // files simultaneously opened at simulator
                          // side.

 
int main(int argc, char* argv[]) {
   
  {
    timeline t("args");
    kwd::type("usr", "Scalar", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    kwd::type("A", "Scalar", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    kwd::type("B", "Scalar", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    kwd::type("C", "Scalar", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    
    "A" << fx::random() |  kwd::use("walltime", WALLLTIME);
    "B" << fx::random() |  kwd::use("walltime", WALLLTIME);
    "C" << fx::random() |  kwd::use("walltime", WALLLTIME);

    // Rules for A, B and C can be applied forever immediately, since
    // walltime it -1 and no arguments are needed for the
    // computation. Nevertheless, as we are in a timeline where usr
    // needs a user input, a timestep in that timeline cannot be
    // completed if usr value is not provided.
  }
  
  {
    timeline t("results");
    kwd::type("min", "Scalar", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    kwd::type("max", "Scalar", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    
    "min" << fx::min({"A", "B", "C"}) |  kwd::use("walltime", WALLLTIME);
    "max" << fx::max({"A", "B", "C"}) |  kwd::use("walltime", WALLLTIME);
  }
  
 
  return 0;
}
