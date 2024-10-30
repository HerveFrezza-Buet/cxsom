#include <cxsom-builder.hpp>
#include <fstream>

#define CACHE        2
#define TRACE    10000
#define OPENED    true
#define FORGET       0
#define WALLTIME    -1 // Rules can be applied forever

#define INPUT_BUF_SIZE 20000
// We feed inputs by bunches of 5000 values (see feed.py). We take a
// large buffer size for storing them, in case of refill while the
// computation is running.

#define MAP_SIZE 20 // We use a 20x20 map here, since we handle 2D maps.

// cxsom declarations
using namespace cxsom::rules;
context* cxsom::rules::ctx = nullptr;


int main(int argc, char* argv[]) {
  context c(argc, argv);

  // Let us gather all parameter definitions here.
  kwd::parameters p_main, p_match, p_learn, p_external, p_contextual, p_global;
  p_main       | kwd::use("epsilon", 0), kwd::use("walltime", WALLTIME);
  p_match      | p_main, kwd::use("sigma", .1);
  p_learn      | p_main, kwd::use("alpha", .1), kwd::use("r", .15);
  p_external   | p_main;
  p_contextual | p_main;
  p_global     | p_main, kwd::use("random-bmu", 1);

  
  auto archi = cxsom::builder::architecture();
  // We use default timelines:
  // - wgt : for weights
  // - rlx : for relaxing variables
  // - out : for BMUs (outputs)

  // We can declare cxsom variables, keeping a reference (input here)
  // for further manipulations. This is usefull for variables that are
  // not computed automatically by builder functions.
  auto input = cxsom::builder::variable("in",                           // We add a timeline "in" for the inputs.
					cxsom::builder::name("xi"),     // The name.
					"Array=2",                      // The type.
					CACHE, INPUT_BUF_SIZE, OPENED); // Same as in cxsom.

  // Here, we decide that this piece of codes is where the input variable has to be defined.
  input->definition();
  
  // Architectures are made of maps. Here, we create a map
  // description, that could be used in several architectures.
  auto map = cxsom::builder::map::make_2D("SOM");

  auto map_settings = cxsom::builder::map::make_settings();
  // The settings has no setting requests yet. Let us define what we want to set.
  map_settings.map_size            = MAP_SIZE;
  map_settings.cache_size          = CACHE;
  map_settings.internals_file_size = FORGET; // This is the history length of internal computation (activities, relaxing BMU...)
  map_settings.weights_file_size   = TRACE;  // This is the history length of weights.
  map_settings.kept_opened         = OPENED;
  map_settings                     = {p_external, p_contextual, p_global};

  // There are many ways to declare external inputs, this one is the
  // simplest, but other ones enable to customize the computation. See
  // the documentation.
  map->external(input,
		fx::match_gaussian, p_match,
		fx::learn_triangle, p_learn); 
  
  // Here, the architecture is made of a single map.
  archi << map;
  *archi = map_settings; // We apply the settings

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
