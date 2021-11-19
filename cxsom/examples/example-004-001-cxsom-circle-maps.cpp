#include <cxsom-rules.hpp>

// This is for further convenience
using namespace cxsom::rules;

// This is a mandatory declaration, it implements some global variable. 
context* cxsom::rules::ctx = nullptr;

// This implements 2 consensus driven maps. Note the use of a specific
// update rule for initialization (we use <= instead of <<).



#define EPSILON           1e-3
#define NB_MAX_RELAXATION 100

#define MAP_SIZE          500

#define RELAX_STEP        .01

#define E_MATCH_SIGMA     .2
#define C_MATCH_SIGMA     .2
#define SIGMA_CONV        .01

#define ALPHA             .05
#define H_RADIUS_E        .25
#define H_RADIUS_C        .075 



#define CACHE       2
#define TRACE   10000
#define OPENED   true
#define FORGET      0

#define WALLTIME -1 // Infinite walltime

void build_map(const std::string& prefix,
	       unsigned int map_size) {
  name_space ns(prefix);

  kwd::parameters p_main, p_learn, p_converge;
  p_main     | kwd::use("walltime", WALLTIME), kwd::use("epsilon", EPSILON);
  p_learn    | p_main, kwd::use("alpha", ALPHA);
  p_converge | kwd::use("deadline", NB_MAX_RELAXATION), kwd::use("random_bmu", 1), kwd::use("sigma", SIGMA_CONV), kwd::use("delta", RELAX_STEP);

  {
    timeline t("wgt"); // Timeline for weight updates.

    // External weights type declaration.
    kwd::type("We", std::string("Map1D<Scalar>=") + std::to_string(map_size), CACHE, TRACE, true);
    // Contextual weights type declaration.
    kwd::type("Wc", std::string("Map1D<Pos1D>=" ) + std::to_string(map_size), CACHE, TRACE, true);

    // Init
    kwd::at("We", 0) << fx::random();
    kwd::at("Wc", 0) << fx::random();    
    
    // Learning
    "We" << fx::learn_triangle(kwd::var("init", "Input"),    kwd::prev("We"), kwd::var("rlx", "BMU"))      | p_learn, kwd::use("r", H_RADIUS_E);    
    "Wc" << fx::learn_triangle(kwd::var("rlx", "remoteBMU"), kwd::prev("Wc"), kwd::var("rlx", "BMU"))      | p_learn, kwd::use("r", H_RADIUS_C);   
  }
  
  {
    timeline t("init"); // Timeline for initialization.
    kwd::type("Ae", std::string("Map1D<Scalar>=") + std::to_string(map_size), CACHE, FORGET, true);
    
    kwd::at("Ae", 0)  << fx::clear()                                                                       | kwd::use("value",  0 );
    "Ae"              << fx::match_gaussian("Input", kwd::prev(kwd::var("wgt", "We")))                     | p_main, kwd::use("sigma", E_MATCH_SIGMA);
  }

  {
    timeline t("rlx"); // Timeline for relaxation.
    
    // Activations
    kwd::type("Ac"       , kwd::type_of({"init", "Ae"}), CACHE, FORGET, true);
    kwd::type("A"        , kwd::type_of({"init", "Ae"}), CACHE, FORGET, true);
    // BMUs and inputs.
    kwd::type("BMU"      , "Pos1D"                     , CACHE, FORGET, true);
    kwd::type("remoteBMU", "Pos1D"                     , CACHE, FORGET, true);
    // Convergence counter (not mandatory)
    kwd::type("Cvg"      , "Scalar"                    , CACHE, TRACE,  true);

    // Rules
    kwd::at("Ac",  0) << fx::clear()                                                                       | kwd::use("value",  0 );
    kwd::at("A",   0) << fx::clear()                                                                       | kwd::use("value",  0 );
    kwd::at("BMU", 0) << fx::clear()                                                                       | kwd::use("value", .5 );
    "Cvg" << fx::converge({kwd::data("Ac"), "A", "BMU"})                                                   | kwd::use("walltime", WALLTIME);
    "Ac"  << fx::match_gaussian("remoteBMU", kwd::prev(kwd::var("wgt", "Wc")))                             | p_main, kwd::use("sigma", C_MATCH_SIGMA);
    "A"   << fx::merge(kwd::var("init", "Ae"), "Ac")                                                       | p_main, kwd::use("beta",  .5 );

    // BMU is part of a relaxation process, but we want it to be
    // initialized from Ae at first.  The updating rule for "BMU" is
    // thus twofold: an initialization and a 'after initialization'
    // update. We use <= for the first update definition (init) and
    // <<, as previously, for the general case. These kind of updates
    // (<=) are those who appear in the '*-inits.dot' files when the
    // graph of computation is generated.
    "BMU" <= fx::conv_argmax(kwd::var("init", "Ae"))                                                       | p_main, kwd::use("sigma", .01);
    "BMU" << fx::toward_conv_argmax("A", "BMU")                                                            | p_main, p_converge;
    
  }
  
}

int main(int argc, char* argv[]) {
  context c(argc, argv);

  build_map("X", MAP_SIZE); // Infinite walltime 
  build_map("Y", MAP_SIZE); // Infinite walltime

  {
    timeline t("input");

    // If you are sure that the inputs already exist, you do not have
    // to specify the cache_size and file_size.
    /* 
       kwd::type("X", "Scalar", true); 
       kwd::type("Y", "Scalar", true); 
    */

    // Here, we create them and set the first instant.
    kwd::type("X", "Scalar", CACHE, TRACE,  true); 
    kwd::type("Y", "Scalar", CACHE, TRACE,  true); 
    kwd::at("X", 0) << fx::clear()                                                                        | kwd::use("value",  0 );
    kwd::at("Y", 0) << fx::clear()                                                                        | kwd::use("value",  0 );
    
    kwd::rename({"init", "X/Input"}, "X");
    kwd::rename({"init", "Y/Input"}, "Y");
  }

  kwd::rename({"rlx", "X/remoteBMU"}, {"rlx", "Y/BMU"});
  kwd::rename({"rlx", "Y/remoteBMU"}, {"rlx", "X/BMU"});

  return 0;
}
