#include <cxsom-builder.hpp>
#include <fstream>

#define CACHE       2
#define TRACE   10000
#define OPENED   true
#define FORGET      0
#define WALLTIME   -1 // Infinite walltime

#define MAP_SIZE 500

// cxsom declarations
using namespace cxsom::rules;
context* cxsom::rules::ctx = nullptr;


int main(int argc, char* argv[]) {
  context c(argc, argv);
  
  auto archi = cxsom::builder::architecture();
  
  kwd::parameters p_main, p_match, p_learn, p_external, p_contextual, p_global;
  p_main       | kwd::use("walltime", WALLTIME), kwd::use("epsilon", 0);
  p_match      | p_main, kwd::use("r", .3);
  p_learn      | p_main, kwd::use("alpha", .05), kwd::use("r", .05);
  p_external   | p_main;
  p_contextual | p_main;
  p_global     | p_main, kwd::use("random-bmu", 1), kwd::use("sigma", .0125);

  auto map_settings = cxsom::builder::map::make_settings();
  map_settings.map_size      = MAP_SIZE;
  map_settings.cache_size    = CACHE;
  map_settings.file_size     = TRACE;
  map_settings.kept_opened   = OPENED;
  map_settings               = {p_external, p_contextual, p_global};
  map_settings.bmu_file_size = TRACE; 
  
  auto input = cxsom::builder::variable("in", cxsom::builder::name("obs"), "Scalar", CACHE, TRACE, OPENED);
  input->definition();

  auto map = cxsom::builder::map::make_1D("recSOM");

  map->external(input, fx::match_triangle, p_match, fx::learn_triangle, p_learn);
  map->external(map,   fx::match_triangle, p_match, cxsom::builder::timestep::previous(), fx::learn_triangle, p_learn);
   
  archi << map;

  *archi = map_settings; // We apply the settings to all maps (ok... a single map here).
  archi->realize();

  // Once the architecture is realized, we can add specific
  // inits. Here, the first step cannot be computed from scratch, since
  // the second external input is the map at t-1 (previous). So we
  // have to initialize the first computation for map ouput at time 0.
  map->internals_random_at(0);
  
  std::ofstream dot_file("recsom-archi.dot");
  dot_file << archi->write_dot;
  
  
  return 0;
}
