#include <cxsom-rules.hpp>

// This is for further convenience
using namespace cxsom::rules;

// This is a mandatory declaration, it implements some global variable. 
context* cxsom::rules::ctx = nullptr;

#define WALLTIME 500
#define EPSILON  1e-8


// The idea here is to count the number of steps required by the
// variables in the relax timeline to get stabilized. We do not need
// to store the values, except the countings. To implement this, we
// use the variables only in the cache (1-sized cache is enough here),
// but 0 filesize is used to avoid saving intermediate values.

#define FORGET          0 // 0-sized filebuf stores no data history on the disk (computation is forgotten).
#define KEEP_TRACE   1000 // We keep at most the 1000 last samples.
#define CACHE_SIZE      1 // This is the number of time instants kept in memory for each variable.

int main(int argc, char* argv[]) {
  context c(argc, argv);

  {
    timeline t("init");

    kwd::type("A", "Map1D<Scalar>=1", CACHE_SIZE, FORGET, true);
    kwd::type("B", "Map1D<Scalar>=1", CACHE_SIZE, FORGET, true);
    kwd::type("C", "Map1D<Scalar>=1", CACHE_SIZE, FORGET, true);

    "A" << fx::random()                                                         | kwd::use("walltime", WALLTIME);
    "B" << fx::random()                                                         | kwd::use("walltime", WALLTIME);
    "C" << fx::random()                                                         | kwd::use("walltime", WALLTIME);
  }

  {
    timeline t("relax");

    kwd::data A =  "A";
    kwd::data B =  "B";
    kwd::data C =  "C";
    kwd::type(A,     kwd::type_of({"init", "A"}), CACHE_SIZE,     FORGET, true);
    kwd::type(B,     kwd::type_of({"init", "B"}), CACHE_SIZE,     FORGET, true);
    kwd::type(C,     kwd::type_of({"init", "C"}), CACHE_SIZE,     FORGET, true);
    kwd::type("Cvg", "Scalar",                    CACHE_SIZE, KEEP_TRACE, true);

    A <= fx::copy({"init", "A"});
    B <= fx::copy({"init", "B"});
    C <= fx::copy({"init", "C"});

    A << fx::average({B, C})                                                    | kwd::use("walltime", WALLTIME), kwd::use("epsilon", EPSILON);
    B << fx::average({C, A})                                                    | kwd::use("walltime", WALLTIME), kwd::use("epsilon", EPSILON);
    C << fx::average({A, B})                                                    | kwd::use("walltime", WALLTIME), kwd::use("epsilon", EPSILON);

    // This counts the number of relaxation steps for the current timestep.
    // Provide as argument the variable for which you want to count stabilization steps.
    "Cvg" << fx::converge({A, B, C})                                            | kwd::use("walltime", WALLTIME);
  }

 
  return 0;
}
