#include <cxsom-rules.hpp>

// This is for further convenience
using namespace cxsom::rules;

// This is a mandatory declaration, it implements some global variable. 
context* cxsom::rules::ctx = nullptr;


// In this example, we build a 1D SOM, fed with 1D data. 

// Inputs have to block map computation, i.e. the map won't compute a
// timestep if the input is not there. To have this effect, at a
// timestep t, input and other map update musn't belong to the same
// timeline, whereas they would relax even if input is not set yet.

// The same stands for weights. At timestep t, weights have to be
// updated after activation computation. So they need to live in their
// own timeline.

 
#define CACHE       2
#define TRACE   10000
#define OPENED   true
#define FORGET      0

#define WALLTIME TRACE-1

int main(int argc, char* argv[]) {
  context c(argc, argv);

  // Some pre-defined parameter sets.
  kwd::parameters p_main, p_learn;
  p_main  | kwd::use("walltime", WALLTIME),  kwd::use("epsilon", 1e-5);
  p_learn | p_main, kwd::use("alpha", .1), kwd::use("tol", 1e-5), kwd::use("r", .1);
    

  {
    timeline t("in");                                          // Input timeline
    kwd::type("xi", "Scalar", CACHE, FORGET, OPENED);          // Input type declaration/check.

    "xi" << fx::random() | kwd::use("walltime", WALLTIME);
  }

  {
    timeline t("wgt");                                         // Weight timeline
    kwd::type("W", "Map1D<Scalar>=500", CACHE, TRACE, OPENED); // Weights type declaration/check.

    // At first timestep, we initialize the weights randomly.
    kwd::at("W", 0) << fx::random();

    // At any **other** timestep as those for which we already have a
    // specific rule (here, all except at 0), we update the
    // weights. In order to refer a variable of another timeline, you
    // can provide {"timeline-name", "variable-name"} as an
    // argument. Nevertheless, for some functions, this call is
    // ambiguous. In this case, or systemetically, you can use
    // kwd::var("timeline-name", "variable-name") to do the
    // same. After the pipe, you have parameters.
    "W" << fx::learn_triangle(kwd::var("in", "xi"), kwd::prev("W"), kwd::var("rlx", "BMU"))   | p_learn;

    // The parameters are the following :
    // - walltime: after WALLTIME steps, the pattern dies. This prevents from infinite computation... except if we use -1 !
    // - epsilon : If the update do not significanlty change the value (variation is less than epsilon), it is considered as stable.
    // - tol     : Weight are updated as w += h*alpha*(x - w). If h*alpha <= tol, it is not done for the sake of efficiency.
    // - alpha   : The learning rate.
    // - r       : the radius of the triangle-shaped neighboring kernel.
  }

  {
    timeline t("rlx");   // Relaxation timeline.

    kwd::type("A",   "Map1D<Scalar>=500", CACHE, FORGET, OPENED); // Matching activities type declaration/check.
    kwd::type("BMU", "Pos1D",             CACHE, FORGET, OPENED); // BMU type declaration/check.

    // First timestep is only initialization.
    kwd::at("A",   0) << fx::clear()                                                         | kwd::use("value",  0);
    kwd::at("BMU", 0) << fx::clear()                                                         | kwd::use("value", .5);

    // Computation
    "A"   << fx::match_gaussian(kwd::var("in", "xi"), kwd::prev(kwd::var("wgt", "W")))       | p_main, kwd::use("sigma", .1);
    "BMU" << fx::argmax("A")                                                                 | p_main, kwd::use("random-bmu", 1);

    // The parameters are the following.
    // - sigma (for "A")  : the width of the gaussian tuning curve used for matching.
  }

  return 0;
}

