#include <cxsom-rules.hpp>

// This is for further convenience
using namespace cxsom::rules;

// This is a mandatory declaration, it implements some global variable. 
context* cxsom::rules::ctx = nullptr;

// This has no real meaning in terms of execution. This example is
// rather a cheat sheet gathering all the rules that are available.

#define CACHE_SIZE     10 
#define BUF_SIZE     1000 
#define KEPT_OPENED false 

int main(int argc, char* argv[]) {
  context c(argc, argv);

  {
    timeline t("random");

    kwd::type("A", "Map1D<Scalar>=500", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    "A" << fx::random()                                                                     | kwd::use("walltime", 10);

    // A(t) = uniform(0, 1)
  }
  
  {
    timeline t("random-when");

    kwd::type("A", "Map1D<Scalar>=500", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    kwd::type("B", "Scalar",            CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    "A" << fx::random_when({"B"})                                                           | kwd::use("walltime", 10);
    // "A" << fx::random_when({"B", "C", ...})   is ok as well.

    // A(t) = uniform(0, 1) when B is ready... It is recommended to
    // choose "B" in another timeline, for actually trigger random
    // computation when B is computd.
  }

  {
    timeline t("clear");

    kwd::type("A", "Map1D<Scalar>=500", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    unsigned int at_max = 10;
    for(unsigned int at = 0; at <= at_max; ++at)
      kwd::at("A", at) << fx::clear()                                                       | kwd::use("value", at/(double)at_max);
    
    // for t in [0, 10], A(t) = t/10, t/10 is the "value" parameter or clear.
  }
  
  {
    timeline t("copy");

    kwd::type("A", "Map1D<Scalar>=100", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    kwd::at("A", 0) << fx::random();
    kwd::at("A", 1) << fx::copy(kwd::at("A", 0));

    // A(1) = A(0)
  }
  
  {
    timeline t("min-max");
    
    kwd::type("A",   "Scalar", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    kwd::type("B",   "Scalar", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    kwd::type("C",   "Scalar", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    kwd::type("min", "Scalar", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    kwd::type("max", "Scalar", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    
    "A" << fx::random()               | kwd::use("walltime", 1000);
    "B" << fx::random()               | kwd::use("walltime", 1000);
    "C" << fx::random()               | kwd::use("walltime", 1000);

    "min" << fx::min({"A", "B", "C"}) | kwd::use("walltime", 1000);
    "max" << fx::max({"A", "B", "C"}) | kwd::use("walltime", 1000);
  }
  
  {
    timeline t("average");
    kwd::type("A", "Map1D<Scalar>=100", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    unsigned int nb = 10;
    for(unsigned int at = 0; at < nb; ++at)
      kwd::at("A", at) << fx::random();
    kwd::at("A", nb)   << fx::average(kwd::at("A", 0, nb))                                  | kwd::use("epsilon", 0);       
    kwd::at("A", nb+1) << fx::context_merge(kwd::at("A", 0, nb))                            | kwd::use("epsilon", 0);  // context_merge is average.

    // for t in [0, 10[, A(t) = uniform(0, 1)
    // A(10) = average(A(0), ... A(9))
    // A(11) = average(A(0), ... A(9))
  }
  
  {
    // Here, we want the inputs at step t to trigger the computation
    // of activities at the same step. We will make the activity
    // computation pattern dependent on input availability, but
    // blocked until the input is actually there. This blocking cannot
    // be done if both live in the same timestep, since they will
    // enter into a relaxation process. This is why we need two
    // separate timelines.
    {
      timeline t("match-inputs");
      kwd::type("In",  "Pos1D", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
      "In"  << fx::random()                                                                 | kwd::use("walltime", 15); // We generate 15 inputs only.
    }
    
    {
      timeline t("match-weights");
      
      kwd::type("Wgt",  "Map1D<Pos1D>=100",  CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
      kwd::type("ActT", "Map1D<Scalar>=100", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
      kwd::type("ActG", "Map1D<Scalar>=100", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
      
      // We generate a single random weight map...
      kwd::at("Wgt", 0) << fx::random();

      
      // ... and an activation update that is blocked if the input
      // **in another timeline** are not ready. A large walltime value
      // is thus not an issue.
      
      // A walltime = -1 value, meaning infinite walltime, would have
      // been ok as well.
      "ActT" << fx::match_triangle({"match-inputs", "In"}, kwd::at("Wgt", 0))               | kwd::use("r",     .3), kwd::use("walltime", 1000);
      "ActG" << fx::match_gaussian({"match-inputs", "In"}, kwd::at("Wgt", 0))               | kwd::use("sigma", .3), kwd::use("walltime", 1000);

      // for t >= 0, ActT(t) = match_triangle(match-inputs-In(t), Wgt(0))
      //             ActT(t) = match_gaussian(match-inputs-In(t), Wgt(0))
    }
  }

  {
    {
      timeline t("merge-inputs");
      kwd::type("Ae", "Map1D<Scalar>=100", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
      kwd::type("Ac", "Map1D<Scalar>=100", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);

      kwd::at("Ae", 0) << fx::random();
      unsigned int at_max = 10;
      for(unsigned int at = 0; at <= at_max; ++at)
	kwd::at("Ac", at) << fx::clear()                                                    | kwd::use("value", at/(double)at_max);
      
      // Ae(0) = uniform(0, 1)
      // Ac(t) = t/10,  for t in [0, 10]
    }
    
    {
      timeline t("merge-computation");
      kwd::type("A", "Map1D<Scalar>=100", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);

      "A" << fx::merge({"merge-inputs", "Ae", 0}, {"merge-inputs", "Ac"})                   | kwd::use("beta",  .5), kwd::use("walltime", 1000);

      // for t >= 0, A(t) = merge(merge-inputs-Ae(0), merge-inputs-Ac(t))
    }
  }

  // Now that you got the meaning of the rules, you can rely on the
  // graph generated by this example to understand what computation
  // rules mean. Do not hesitate to comment some parts before
  // generating the graph if it appears messy.

  {
    {
      timeline t("learn-inputs");
      
      kwd::type("Wgt", "Map1D<Scalar>=100", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
      kwd::type("BMU", "Pos1D",             CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
      kwd::type("In",  "Scalar",            CACHE_SIZE, BUF_SIZE, KEPT_OPENED);

      kwd::at("Wgt", 0) << fx::random();
      kwd::at("BMU", 0) << fx::clear()                                                      | kwd::use("value",  .5);
      kwd::at("In",  0) << fx::clear()                                                      | kwd::use("value", 1.0);
    }
    
    {
      timeline t("learn-weights");

      kwd::type("WgtT", kwd::type_of({"learn-inputs", "Wgt"}), CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
      kwd::type("WgtG", kwd::type_of({"learn-inputs", "Wgt"}), CACHE_SIZE, BUF_SIZE, KEPT_OPENED);

      kwd::at("WgtT", 0) << fx::copy(kwd::at(kwd::var("learn-inputs", "Wgt"), 0));
      kwd::at("WgtG", 0) << fx::copy(kwd::at(kwd::var("learn-inputs", "Wgt"), 0));

      "WgtT" << fx::learn_triangle(kwd::at(kwd::var("learn-inputs", "In"), 0),
				   kwd::prev("WgtT"),
				   kwd::at(kwd::var("learn-inputs", "BMU"), 0))             | kwd::use("walltime", 100), kwd::use("alpha", .05), kwd::use("r",     .3), kwd::use("tol", 1e-5), kwd::use("epsilon", .01); 
      "WgtG" << fx::learn_gaussian(kwd::at(kwd::var("learn-inputs", "In"), 0),
				   kwd::prev("WgtG"),
				   kwd::at(kwd::var("learn-inputs", "BMU"), 0))             | kwd::use("walltime", 100), kwd::use("alpha", .05), kwd::use("sigma", .1), kwd::use("tol", 1e-5), kwd::use("epsilon", .01);
    }
  }

  {
    timeline t("bmu");

    // Argmax related stuff have a "deadline" parameter, that stops updated in relaxation.

    kwd::type("Act",        "Map1D<Scalar>=100", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    kwd::type("BMU_noconv", "Pos1D",             CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    kwd::type("BMU_conv",   "Pos1D",             CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    "Act"        << fx::random()                                                            | kwd::use("walltime", 10);
    "BMU_noconv" << fx::argmax("Act")                                                       | kwd::use("walltime", 10), kwd::use("deadline", 100), kwd::use("random-bmu", 1.0), kwd::use("argmax-tol", 1e-8), kwd::use("epsilon", .01);
    "BMU_conv"   << fx::conv_argmax("Act")                                                  | kwd::use("walltime", 10), kwd::use("deadline", 100), kwd::use("random-bmu", 0.0), kwd::use("epsilon", .01), kwd::use("sigma", .05);
  }

  {
    {
      timeline t("toward-weights");
      
      kwd::type("Act", "Map1D<Scalar>=100", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
      kwd::at("Act", 0) << fx::random();
    }
    
    {
      timeline t("toward-bmu");

      kwd::type("BMU", "Pos1D",      CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
      kwd::type("NbSteps", "Scalar", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
      kwd::at("BMU", 0) << fx::clear()                                                      | kwd::use("value", .5);
      "BMU" << fx::toward_argmax(kwd::at(kwd::var("toward-weights", "Act"), 0), "BMU")      | kwd::use("walltime", 100), kwd::use("deadline", 100), kwd::use("random-bmu", 1.0), kwd::use("argmax-tol", 1e-8), kwd::use("epsilon", .01), kwd::use("delta", .05);
      "NbSteps" << fx::converge({kwd::data("BMU")})                                         | kwd::use("walltime", 100);
    }
    
    {
      timeline t("toward-conv-bmu");

      kwd::type("BMU", "Pos1D",      CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
      kwd::type("NbSteps", "Scalar", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
      kwd::at("BMU", 0) << fx::clear()                                                      | kwd::use("value", .5);
      "BMU" << fx::toward_conv_argmax(kwd::at(kwd::var("toward-weights", "Act"), 0), "BMU") | kwd::use("walltime", 100), kwd::use("deadline", 100), kwd::use("random-bmu", 1.0), kwd::use("argmax-tol", 1e-8), kwd::use("sigma", .05), kwd::use("epsilon", .01), kwd::use("delta", .05);
      "NbSteps" << fx::converge({kwd::data("BMU")})                                         | kwd::use("walltime", 100);
    }

    {
      timeline t("value-at");
      kwd::type("Value",      "Array=3",            CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
      kwd::type("Collection", "Map2D<Array=3>=100", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
      kwd::type("Idx",        "Pos2D",              CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
      "Value" << fx::value_at("Collection", "Idx")                                          | kwd::use("walltime", 100); 
    }

    {
      timeline t("pairs");
      kwd::type("ABin",  "Pos2D", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
      kwd::type("ABout", "Pos2D", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
      kwd::type("Ain",   "Pos1D", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
      kwd::type("Aout",  "Pos1D", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
      kwd::type("Bin",   "Pos1D", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
      kwd::type("Bout",  "Pos1D", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
      "ABin" << fx::pair("Aout", "Bout") | kwd::use("walltime", 1000);
      "Ain"  << fx::first("ABout")       | kwd::use("walltime", 1000);
      "Bin"  << fx::second("ABout")      | kwd::use("walltime", 1000);
    }
  }
  

              
  return 0;
}
