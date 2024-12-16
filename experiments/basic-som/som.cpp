#include <cxsom-builder.hpp>
#include <fstream>

#define CACHE        2
#define TRACE    10000
#define OPENED    true
#define FORGET       0
#define FOREVER     -1 // Rules can be applied forever

#define SAVE_PERIOD 500

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
  p_main       | kwd::use("epsilon", 0), kwd::use("walltime", FOREVER);
  p_match      | p_main, kwd::use("sigma", .1);
  p_learn      | p_main, kwd::use("alpha", .1), kwd::use("r", .2);
  p_external   | p_main;
  p_contextual | p_main;
  p_global     | p_main, kwd::use("random-bmu", 0);

  
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

  // Here, we perform the cxsom definition of the variable.
  input->definition();
  
  // Architectures are made of maps. Here, we create a map
  // description (that could be used in several architectures).
  auto map = cxsom::builder::map::make_2D("SOM");

  auto map_settings = cxsom::builder::map::make_settings();
  // The settings has no setting requests yet. Let us define what we want to set.
  map_settings.map_size            = MAP_SIZE;
  map_settings.cache_size          = CACHE;
  map_settings.internals_file_size = FORGET; // This is the history length of internal computation (activities, relaxing BMU...)
  map_settings.weights_file_size   = TRACE;  // This is the history length of weights.
  map_settings.kept_opened         = OPENED; // We keep variable files opened during the computation.
  map_settings.argmax              = fx::argmax;
  map_settings                     = {p_external, p_contextual, p_global};

  // There are many ways to declare external inputs, this one is the
  // simplest, but other ones enable to customize the computation. See
  // the documentation.
  auto layer = map->external(input,
			     fx::match_gaussian, p_match,
			     fx::learn_triangle, p_learn); 
  
  // Here, the architecture is made of a single map.
  archi << map;
  *archi = map_settings; // We apply the settings

  // This has to be done once, when everything is ready. This
  // actually declares all the rules defined in the architecture.
  archi->realize();

  // Updates of weights reads weights at t-1... so timestep 0 cannot
  // be implemente with the general updating rule. Let us specify it as
  // setting all the variables randomly.
  map->internals_random_at(0);

  // It can be nice to save weights every SAVE_PERIOD steps (we have
  // already bufferd the past TRACE weight values).

  // Let us first retrieve the variable containing the weights... from
  // the layer where they are stored.
  auto weights = layer->_W();

  // We declare a new variable for periodical weight save.
  auto wgt_save = cxsom::builder::variable("save",                 // We add a timeline "save".
					   weights->varname,       // We use the same name.
					   weights->type,          // We use the same type.
					   CACHE, TRACE, OPENED);  // Same as in cxsom.
  wgt_save->definition(); // We actually cxsom-define the variable now.

  // Then, we add the rule for saving every SAVE_PERIOD time steps.
  wgt_save->var() << fx::copy(kwd::times(weights->var(), SAVE_PERIOD)) | p_main;             
  
  
  // We describe the architecture in a dot file.
  std::ofstream dot_file("som-archi.dot");
  dot_file << archi->write_dot;
  
  return 0;
}
