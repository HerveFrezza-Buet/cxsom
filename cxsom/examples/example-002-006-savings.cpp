#include <cxsom-rules.hpp>

using namespace cxsom::rules;
context* cxsom::rules::ctx = nullptr;

// This illustrates the use of the times operator to set up periodical
// saving.

// What you can do, once these rules are submitted to the simulator,
// is to feed input-X manually (using example-001-001 or a pycxsom
// script), and then ping the simulator (use the cxsom-ping
// command). You should observe (cxsom-all-instances) that compute-X
// expands in time, following the expansion od input-X, after the
// ping. Moreover, every 10 timesteps, a new savings/X instance
// should be added.

#define CACHE        2
#define TRACE      500
#define OPENED    true
#define FORGET       0

int main(int argc, char* argv[]) {
  context c(argc, argv);

  
  {
    timeline t("input");
    
    kwd::type("X", "Scalar", CACHE, TRACE, true);
    kwd::at("X", 0) << fx::random();
  }
  
  {
    timeline t("compute");
    
    kwd::type("X", "Scalar", CACHE, TRACE, true);
    "X" << fx::copy(kwd::var("input", "X")) | kwd::use("walltime", -1);
  }
  
  {
    timeline t("savings");
    
    kwd::type("X", "Scalar", CACHE, TRACE, true);
    // X[t] is a copy of compute/X[100*t]
    "X" << fx::copy(kwd::times(kwd::var("compute", "X"), 10)) | kwd::use("walltime", -1);
  }
  
  return 0;
}
