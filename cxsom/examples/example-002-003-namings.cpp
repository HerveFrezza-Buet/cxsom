
#include <cxsom-rules.hpp>

// This is for further convenience
using namespace cxsom::rules;

// This is a mandatory declaration, it implements some global variable. 
context* cxsom::rules::ctx = nullptr;

// This example illustrate the use of syntactical (i.e. kwd::*) tools
// for convenient namings of the variables.

#define CACHE_SIZE     10 
#define BUF_SIZE     1000 
#define KEPT_OPENED false 

int main(int argc, char* argv[]) {
  context c(argc, argv);
  
  {
    timeline t("args");

    // kwd::ith adds a number to a variable name.
    
    for(unsigned int i=5; i < 10; ++i)
      kwd::type(kwd::ith("A", i),   "Map1D<Scalar>=1", CACHE_SIZE, BUF_SIZE, KEPT_OPENED); // Defines "args/A-5", ..., "args/A-9"

    // It works with number ranges.
    kwd::type(kwd::ith("B", 5, 10), "Map1D<Scalar>=1", CACHE_SIZE, BUF_SIZE, KEPT_OPENED); // Defines "args/B-5", ..., "args/B-9"
    kwd::type(kwd::ith("C", 5, 10), "Map1D<Scalar>=1", CACHE_SIZE, BUF_SIZE, KEPT_OPENED); // Defines "args/C-5", ..., "args/C-9"

    // kwd::at does the same with instants. Ranges cannot be used at left side.
    for(unsigned int i=5; i < 10; ++i)
      kwd::at(kwd::ith("A", i), 0) << fx::random();
    // args-A-5(t) = uniform(0,1), for t in [5, 10[
    // Let us rather use the notation X@t for X(t) in the following.
  }
  
  {
    timeline t("namings");

    // The keyword "kwd::var" enables to specify another timeline.

    // Left side of rules cannot be ranges. However, ranges can be
    // generated and then iterated.
    auto Bs = kwd::ith(kwd::var("args", "B"), 5, 10); // Defines the "args/B-5", "args/B-6", ...
    for(auto& Bi : Bs) {
      auto Bi_instants = kwd::at(Bi, 0, 3);           // Defines "args/B-i@0", "args/B-i@1"...
      for(auto& B : Bi_instants)
	B << fx::random();
    }

    // The same could have been written in another way, let us do it with C.
    auto C_instants = kwd::at(kwd::var("args", "C"), 0, 3); // Defines the "args/C@0", "args/C@1"...
    for(auto& C_at : C_instants) {
      auto Cs = kwd::ith(C_at, 5, 10); // Defines "args/C-5@at", "args/C-6@at"...
      for(auto& C : Cs)
	C << fx::random();
    }

    // Loops may be useless in operator arguments, since as opposed to
    // left side of rules, some operator can accept data ranges.
    kwd::type("avgB", kwd::type_of(kwd::ith(kwd::var("args", "B"), 5)), CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    "avgB" << fx::average(kwd::ith(kwd::var("args", "B"), 5, 10))                                                    | kwd::use("walltime", 100);

    // kwd::shift work as kwd::at, expressing a relative timestep.
    auto C = kwd::ith(kwd::var("args", "C"), 5);
    kwd::type("avgC", kwd::type_of(C), CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    "avgC" << fx::average(kwd::shift(C, -5, 0))                                                                      | kwd::use("walltime", 100);

    // kwd::prev is a -1 shift.
    kwd::type("avgA", kwd::type_of(kwd::ith(kwd::var("args", "A"), 5)), CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    "avgA" << fx::average(kwd::prev(kwd::ith(kwd::var("args", "A"), 5, 10)))                                         | kwd::use("walltime", 100);
  }

  {
    timeline t("namespaces");

    // If a variable name *** do not start with / ***, its name is
    // prefixed with current namespaces names. Otherwise, it is used
    // directly as the full variable name. Namespaces concern variable
    // names, not the timelines name that is the first prefix of the
    // whole variable designation.
    kwd::type("X", "Scalar", CACHE_SIZE, BUF_SIZE, KEPT_OPENED); // namespaces/X
    {
      name_space ns("ns1");
      kwd::type("X", "Scalar", CACHE_SIZE, BUF_SIZE, KEPT_OPENED); // namespaces/ns1/X
      {
	name_space ns("ns2");
	kwd::type("X", "Scalar", CACHE_SIZE, BUF_SIZE, KEPT_OPENED); // namespaces/ns1/ns2/X
	"X" << fx::random()                                                                                          | kwd::use("walltime", 100);
      }
      kwd::type("Y", "Scalar", CACHE_SIZE, BUF_SIZE, KEPT_OPENED); // namespaces/ns1/Y
      "X" << fx::average({"Y", "/X"})                                                                                | kwd::use("walltime", 100);
      "Y" << fx::average({"X", "ns2/X"})                                                                             | kwd::use("walltime", 100);
    }
    
  }

  // Have a look at the generated graph to understand which variables
  // names and which rules are defined with such expressions.
              
  return 0;
}
