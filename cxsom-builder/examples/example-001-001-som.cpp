#include <cxsom-builder.hpp>
#include <fstream>

#define CACHE       2
#define TRACE   10000
#define OPENED   true
#define FORGET      0
#define WALLTIME TRACE-1

#define MAP_SIZE 500

// cxsom declarations
using namespace cxsom::rules;
context* cxsom::rules::ctx = nullptr;


int main(int argc, char* argv[]) {
  context c(argc, argv);

  // Let us gather all parameter definitions here.
  kwd::parameters p_main, p_match, p_learn, p_external, p_contextual, p_global;
  p_main       | kwd::use("walltime", WALLTIME),  kwd::use("epsilon", 0);
  p_match      | p_main, kwd::use("sigma", .1);
  p_learn      | p_main, kwd::use("alpha", .1), kwd::use("r", .1);
  p_external   | p_main;
  p_contextual | p_main;
  p_global     | p_main, kwd::use("random-bmu", 1);

  
  auto archi = cxsom::builder::architecture();
  archi->timelines.weights    = "wgt"; // This is the default
  archi->timelines.relaxation = "rlx"; // This is the default
  archi->timelines.output     = "out"; // This is the default
  

  // We can declare cxsom variables, keeping a reference (input here)
  // for further manipulations. This is usefull for variables that are
  // not computed automatically by builder functions.
  auto input = cxsom::builder::variable("in",                              // The timeline.
					cxsom::builder::name("1D") / "xi", // The name.
					"Scalar",                          // The type.
					CACHE, FORGET, OPENED);            // Same as in cxsom.

  // Here, we decide that this piece of codes is where the input variable has to be defined.
  input->definition();

  // Moreover, we will fill the input randomly in this file. Usually,
  // feeding with inputs is done in another process.
  kwd::var(input->timeline, input->varname) << fx::random() | kwd::use("walltime", WALLTIME);
  
  // Architectures are made of maps. Here, we create a map
  // description, that could be used in several architectures.
  auto map = cxsom::builder::map1D("SOM", MAP_SIZE, CACHE, TRACE, OPENED);
  map->argmax = fx::argmax; // default is fx::conv_argmax;
  // map->toward_argmax is not used since we do not have context here.

  // We set the parameters for each computational step...
  map->p_external   = p_external;
  map->p_contextual = p_contextual;
  map->p_global     = p_global;

  // ... but there is a simpler way to write the same:
  *map = {p_external, p_contextual, p_global};

  // The output variable (i.e. the bmu) is forgotten by default. You
  // can set a filesize if you want to keep memory of the selected
  // BMUs.
  map->bmu_file_size = 1000; // Default is 0.

  // There are many ways to declare external inputs, this one is the
  // simplest, but other ones enable to customize the computation. See
  // the documentation.
  map->external(input,
		fx::match_gaussian, p_match,
		fx::learn_triangle, p_learn); 
  
  // Here, the architecture is made of a single map.
  archi << map;

  // This has to be done once, when everything is ready. This
  // actually declares all the rules defined in the architecture.
  archi->realize();

  // Updates of weights reads weights at t-1... so timestep 0 cannot
  // be implement with the general updating rule. Let us specify it as
  // setting all the variables randomly.
  map->internals_random_at(0);
  
  
  // We describe the architecture in a dot file.
  std::ofstream dot_file("som-archi.dot");
  dot_file << archi->write_dot;
  
  
  return 0;
}
