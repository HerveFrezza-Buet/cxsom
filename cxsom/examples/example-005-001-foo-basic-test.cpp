#include <cxsom-rules.hpp>               // This is for basic cxsom stuff.
#include "example-005-001-foo-rules.hpp" // This is for foo stuff.

#include <sstream>
#include <iterator>

// This is for further convenience
using namespace cxsom::rules;

// This is a mandatory declaration, it implements some global variable. 
context* cxsom::rules::ctx = nullptr;

#define CACHE        2
#define TRACE      100
#define OPENED    true
#define FORGET       0
#define MAP_SIZE   100

#define WALLTIME    20
#define EPSILON   1e-3
#define NB_INPUTS   10

int main(int argc, char* argv[]) {
  context c(argc, argv);
  
  kwd::parameters p_main;
  p_main     | kwd::use("walltime", WALLTIME), kwd::use("epsilon", EPSILON);

  
  std::string data_type;
  {
    std::ostringstream ostr;
    ostr << "Map1D<Scalar>=" << MAP_SIZE;
    data_type = ostr.str();
  }

  
  {
    timeline t("in"); // Timeline for inputs.

    for(unsigned int i=0; i < NB_INPUTS; ++i) {
      auto Xi = kwd::ith("X", i);
      kwd::type(Xi, data_type, CACHE, TRACE, true);
      Xi << fx::random() | p_main;
    }
  }

  {
    timeline t("foo"); // Timeline for foo.

    kwd::type("Y", data_type, CACHE, TRACE, true);
    
    std::vector<kwd::data> args;
    auto out_arg = std::back_inserter(args);
    for(unsigned int i=0; i < NB_INPUTS; ++i) *(out_arg++) = kwd::ith(kwd::var("in", "X"), i);

    bool generate_type_error = false; // Set as true for testing type checking errors.
    if(generate_type_error) {
      kwd::type("Z", "Scalar", CACHE, TRACE, true);
      *(out_arg++) = "Z";
    }

    "Y" << foo::fx::mid_bound(args) | p_main;
  }

  return 0;
}
