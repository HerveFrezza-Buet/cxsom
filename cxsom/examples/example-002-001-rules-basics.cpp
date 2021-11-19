#include <cxsom-rules.hpp>

// This is for further convenience
using namespace cxsom::rules;

// This is a mandatory declaration, it implements some global variable. 
context* cxsom::rules::ctx = nullptr;

// This example introduces the simulation rule definitions.

#define WALLTIME      100 // This ends the pattern expansion
#define CACHE_SIZE     10 // This is the cache size at simulator/processor side
#define BUF_SIZE     1000 // This is the file circular buffer size.
#define KEPT_OPENED false // Keeping the file opened saves time but
			  // there is a system limit of the number of
			  // files simultaneously opened at simulator
			  // side.

int main(int argc, char* argv[]) {
  // The compiled program can be called without arguments. It displays
  // an help in that case. There are requires arguments, and, after
  // the sequence '--', user defined arguments. For example
  //   ./my_prog send localhost 10000 -- arg1 arg2 arg3
  // adds 3 arguments "arg1 arg2 arg3" to the basic call "./my_prog send localhost 10000".
  //
 
  context c(argc, argv);

  // User arguments after "--" are stored in the c.user_argv
  // vector. You can test it and display an error message. Here, no
  // user arguments are expected. We test this.
  if(c.user_argv.size() != 0) {
    std::cout << "You are not allowed to provide user arguments." << std::endl;
    c.notify_user_argv_error(); // Do this to avoid irrelevant default behavior when your program exists.
    return 0;
  }   

  {
    timeline t("main"); // We work in the timeline "main".
    
    {
      name_space ns("samplers");
      // All names that do not start with '/' are prefixed with this namespace.
      
      // If variables already exist at server side, declaring types is
      // not mandatory, or CACHE_SIZE, BUF_SIZE can be ommitted.
      kwd::type("B", "Map1D<Scalar>=100", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
      kwd::type("C", kwd::type_of("B")  , CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
      kwd::type("D", kwd::type_of("B")  , CACHE_SIZE, BUF_SIZE, KEPT_OPENED);

      // "B" means 'all timesteps for variable B. These rules are not
      // a specific update (i.e. telling how to compute the value of
      // "B" at a specific timestep, as kwd::at("B", 132) << ... would
      // mean) but a so called "update pattern", that will apply to
      // each time instant t for updating kwd::at("B", t). The
      // walltime limits the extention of the pattern to an infinite
      // future.
      "B" << fx::random()                                                                                                | kwd::use("walltime", WALLTIME);
      "C" << fx::random()                                                                                                | kwd::use("walltime", WALLTIME);
      "D" << fx::random()                                                                                                | kwd::use("walltime", WALLTIME);
    }
    
    {
      name_space ns("averagers");
      
      kwd::type("A", "Map1D<Scalar>=100", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
      kwd::type("B", kwd::type_of("A"),   CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
      kwd::type("C", kwd::type_of("A"),   CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
      kwd::type("D", kwd::type_of("A"),   CACHE_SIZE, BUF_SIZE, KEPT_OPENED);

      // Here, we say that the values at times 0, 1 and 2 for the 4
      // variables are initialized randomly. Such exlicit
      // initialization replaces any initialization that would come
      // from the application of an update pattern.
      for(int i = 0; i < 3; ++i) {
	kwd::at("A", i) << fx::clear()                                                                                   | kwd::use("value", 0.5);
	kwd::at("B", i) << fx::clear()                                                                                   | kwd::use("value", 0.5);
	kwd::at("C", i) << fx::clear()                                                                                   | kwd::use("value", 0.5);
	kwd::at("D", i) << fx::clear()                                                                                   | kwd::use("value", 0.5);
      }

      // We define update patterns for timesteps t>=3.
      
      "A" << fx::average({kwd::shift("B", -1), kwd::shift("C", -1), kwd::shift("D", -1)})                                | kwd::use("walltime", WALLTIME);
      // means : forall t, A(t) = average(B(t-1), C(t-1), D(t-1))
      
      "B" << fx::average({kwd::shift("/samplers/B", -1), kwd::shift("/samplers/B", -2), kwd::shift("/samplers/B", -3)})  | kwd::use("walltime", WALLTIME);
      "C" << fx::average({kwd::shift("/samplers/C", -1), kwd::shift("/samplers/C", -2), kwd::shift("/samplers/C", -3)})  | kwd::use("walltime", WALLTIME);
      "D" << fx::average({kwd::shift("/samplers/D", -1), kwd::shift("/samplers/D", -2), kwd::shift("/samplers/D", -3)})  | kwd::use("walltime", WALLTIME);
    }
  }

  // Do not forget to have a look at the computation graphs generated by this code when the 'graph' command is provided.

  return 0;
}
