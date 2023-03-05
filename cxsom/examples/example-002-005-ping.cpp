#include <cxsom-rules.hpp>

using namespace cxsom::rules;
context* cxsom::rules::ctx = nullptr;

// This illustrates the ping mechanism. This example sets the first
// value of an input variable input-X, and defines a pattern telling
// that compute-X should be a copy of input-X... until the end of time
// (walltime = -1). The process is blocked by input starvation, here
// after t=0.

// What you can do, once these rules are submitted to the simulator,
// is to feed input-X manually (using example-001-001 or a pycxsom
// script), and then ping the simulator (use the cxsom-ping
// command). You should observe (cxsom-all-instances) that compute-X
// expands in time, following the expansion od input-X, after the
// ping.

#define CACHE       2
#define TRACE      10
#define OPENED   true
#define FORGET      0

int main(int argc, char* argv[]) {
  context c(argc, argv);

  
  {
    timeline t("input");
    
    kwd::type("X", "Scalar", CACHE, TRACE, true);
    kwd::at("X", 0) << fx::random();
  }
  
  {
    timeline t("compute");
    
    kwd::type("X", "Scalar", CACHE, FORGET, true);
    "X" << fx::copy(kwd::var("input", "X")) | kwd::use("walltime", -1);
  }
  
  return 0;
}
