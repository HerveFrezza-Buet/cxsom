#include <cxsom-builder.hpp>
#include <fstream>

#define CACHE        2
#define TRACE    10000
#define OPENED    true
#define FORGET       0
#define FOREVER     -1 // Rules can be applied forever

#define INPUT_BUF_SIZE 20000

#define MAP_SIZE 500


// cxsom declarations
using namespace cxsom::rules;
context* cxsom::rules::ctx = nullptr;

/**
 * We set up a re-entrant SOM. It is a 1D map, fed with scalar values.
 */

int main(int argc, char* argv[]) {
  context c(argc, argv);

  // Let us gather all parameter definitions here.
  kwd::parameters p_main, p_match, p_learn, p_external, p_contextual, p_global;
  p_main       | kwd::use("epsilon", 0), kwd::use("walltime", FOREVER);
  p_learn      | p_main, kwd::use("alpha", .05), kwd::use("r", .05);
  p_match      | p_main, kwd::use("r", .3);
  p_external   | p_main;
  p_contextual | p_main;
  p_global     | p_main, kwd::use("random-bmu", 0);

  
  auto archi = cxsom::builder::architecture();
  
  auto input = cxsom::builder::variable("in", cxsom::builder::name("xi"), "Scalar",  
					CACHE, INPUT_BUF_SIZE, OPENED); 
  input->definition();
  
  auto map = cxsom::builder::map::make_1D("recSOM");

  auto map_settings = cxsom::builder::map::make_settings();
  map_settings.map_size            = MAP_SIZE;
  map_settings.cache_size          = CACHE;
  map_settings.internals_file_size = FORGET;
  map_settings.weights_file_size   = TRACE;
  map_settings.bmu_file_size       = TRACE; // We need the history of the map outputs here.
  map_settings.kept_opened         = OPENED; 
  map_settings.argmax              = fx::argmax;
  map_settings.external_merge      = fx::average; // This is the default.
  map_settings                     = {p_external, p_contextual, p_global};

  // This is the external layer, handling the external scalar input.
  map->external(input,
		fx::match_triangle, p_match,
		fx::learn_triangle, p_learn);

  // We need another layer, handling the BMU position of our map at
  // t-1. It is an external input as well, since the BMU at t-1 isn't
  // part of relaxation (there is no relaxation at all, here).
  map->external(map,
		fx::match_triangle, p_match,
		cxsom::builder::timestep::previous(), fx::learn_triangle, p_learn);
  
  
  archi << map;
  *archi = map_settings;
  archi->realize();

  map->internals_random_at(0);
  
  std::ofstream dot_file("recsom-archi.dot");
  dot_file << archi->write_dot;
  
  return 0;
}
