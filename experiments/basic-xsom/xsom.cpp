/*

  Here, we consider 2 connected maps with external inputs. In order to
  analyze inner relaxation mechanisms, we can send the rules in
  different modes:
  - main   : This is the usual mode.
  - relax  : This generates rules that enable to expand the relaxation process on successive timesteps.
  - frozen : This is the mode without learning.
  - test   : This just defines external variable (architecture is not used)

  Having these 4 modes is used here as an illustration, the main mode does the usual job.
*/

#include <cxsom-builder.hpp>
#include <fstream>
#include <sstream>


#define CACHE              2
#define UNDEFINED_TRACE 1000
#define FROZEN_TRACE    1000
#define OPENED          true
#define OPEN_AS_NEEDED false
#define FORGET             0
#define FOREVER           -1 // Infinite walltime
#define DEADLINE         100 // The maximal number of relaxation steps allowed.

// This is used for frozen or test mode. Partial recording puts some
// internal variables with a 0-sized file.
#define FULL_RECORD     true
#define PARTIAL_RECORD false

#define MAP_SIZE         500

// cxsom declarations
using namespace cxsom::rules;
context* cxsom::rules::ctx = nullptr;

// This function builds up the two maps.
auto make_architecture(bool define_inputs, unsigned int trace) {

  auto archi = cxsom::builder::architecture();
  
  kwd::parameters p_main, p_match, p_learn, p_learn_e, p_learn_c, p_external, p_contextual, p_global;
  p_main       | kwd::use("walltime", FOREVER), kwd::use("epsilon", 0);
  p_match      | p_main,  kwd::use("sigma", .2);
  p_learn      | p_main,  kwd::use("alpha", .1);
  p_learn_e    | p_learn, kwd::use("r", .2 );
  p_learn_c    | p_learn, kwd::use("r", .02);
  p_external   | p_main;
  p_contextual | p_main;
  p_global     | p_main,  kwd::use("random-bmu", 1), kwd::use("beta", .5), kwd::use("delta", .02), kwd::use("deadline", DEADLINE);
  
  auto map_settings = cxsom::builder::map::make_settings();
  map_settings.map_size          = MAP_SIZE;
  map_settings.cache_size        = CACHE;
  map_settings.weights_file_size = trace;
  map_settings.kept_opened       = OPENED;
  map_settings                   = {p_external, p_contextual, p_global};
  map_settings.argmax            = fx::argmax;
  map_settings.toward_argmax     = fx::toward_argmax;

  auto X = cxsom::builder::variable("in", cxsom::builder::name("X"), "Scalar", CACHE, trace, OPENED);
  auto Y = cxsom::builder::variable("in", cxsom::builder::name("Y"), "Scalar", CACHE, trace, OPENED);
  if(define_inputs) {
    X->definition();
    Y->definition();
  }
  
  auto Xmap = cxsom::builder::map::make_1D("X");
  auto Ymap = cxsom::builder::map::make_1D("Y");  
  
  Xmap->external  (X,    fx::match_gaussian, p_match, fx::learn_triangle, p_learn_e);
  Xmap->contextual(Ymap, fx::match_gaussian, p_match, fx::learn_triangle, p_learn_c);
  Ymap->external  (Y,    fx::match_gaussian, p_match, fx::learn_triangle, p_learn_e);
  Ymap->contextual(Xmap, fx::match_gaussian, p_match, fx::learn_triangle, p_learn_c);

  archi << Xmap << Ymap;
  *archi = map_settings;

  // This tells to att a relaxation counter, for counting at each
  // timestep how many relaxations have been performed to reach
  // stability.
  archi->relax_count = "Cvg"; 
  return archi;
}

// We implement these 4 modes.
enum class Mode : char {Main, Relax, Frozen, Test};

int main(int argc, char* argv[]) {
  context c(argc, argv);
  Mode mode = Mode::Main;
  std::string input_prefix;
  std::string analysis_prefix;
  unsigned int weight_time;
  unsigned int trace = 0;


  // We analyse the arguments and identify the mode.
  std::ostringstream prefix;
  for(const auto& arg : c.argv) prefix << arg << ' ';
  prefix << "-- ";
    
  if(c.user_argv.size() == 0) {
    std::cout << "You have to provide user arguments." << std::endl
	      << "e.g:" << std::endl
	      << "  " << prefix.str() << "main <trace>                    <-- sends the main rules." << std::endl
	      << "  " << prefix.str() << "relax <timeline-prefix> <time>  <-- sends relaxation rules for weights at time." << std::endl
	      << "  " << prefix.str() << "test <input-timeline-prefix>    <-- sends 'frozen' input declaration rules." << std::endl
	      << "  " << prefix.str() << "frozen <input-timeline-prefix> <timeline-prefix> <time> <-- sends 'frozen' rules for weights at time." << std::endl;
    c.notify_user_argv_error(); 
    return 0;
  }

  // We parse extra cxsom arguments (after the -- separator).

  if(c.user_argv[0] == "main") {
    if(c.user_argv.size() != 2) {
      std::cout << "The 'main' mode expects 1 arguments"  << std::endl;
      c.notify_user_argv_error(); 
      return 0;
    }
    mode  = Mode::Main;
    trace = stoul(c.user_argv[1]);
  }
  else if(c.user_argv[0] == "relax") {
    if(c.user_argv.size() != 3) {
      std::cout << "The 'relax' mode expects 2 arguments"  << std::endl;
      c.notify_user_argv_error(); 
      return 0;
    }
    mode = Mode::Relax;
    analysis_prefix = c.user_argv[1];
    weight_time = stoul(c.user_argv[2]);
  }
  else if(c.user_argv[0] == "frozen") {
    if(c.user_argv.size() != 4) {
      std::cout << "The 'frozen' mode expects 3 arguments"  << std::endl;
      c.notify_user_argv_error(); 
      return 0;
    }
    mode = Mode::Frozen;
    input_prefix = c.user_argv[1];
    analysis_prefix = c.user_argv[2];
    weight_time = stoul(c.user_argv[3]);
  }
  else if(c.user_argv[0] == "test") {
    if(c.user_argv.size() != 2) {
      std::cout << "The 'test' mode expects 1 arguments"  << std::endl;
      c.notify_user_argv_error(); 
      return 0;
    }
    mode = Mode::Test;
    input_prefix = c.user_argv[1];
  }
  
  else {
    std::cout << "Bad user arguments." << std::endl;
    c.notify_user_argv_error(); 
    return 0;
  }
  

  // Now, according to the mode, let us send rules.

  if(mode == Mode::Main) {
    auto archi = make_architecture(true, trace);
    archi->realize();
    for(auto map : archi->maps) map->internals_random_at(0);

    if(c.argv[1] == "graph" || c.argv[1] == "graph-full") {
      std::ofstream dot_file("archi.dot");
      dot_file << archi->write_dot;
    }
  }

  
  if(mode == Mode::Relax) {
    auto archi = make_architecture(false, UNDEFINED_TRACE);
    auto X = cxsom::builder::variable(analysis_prefix + "-in", cxsom::builder::name("X"), "Scalar", 1, 1, OPENED);
    auto Y = cxsom::builder::variable(analysis_prefix + "-in", cxsom::builder::name("Y"), "Scalar", 1, 1, OPENED);
    X->definition();
    Y->definition();
    
    archi->expand_relax({std::string(), analysis_prefix, CACHE, DEADLINE + 1, OPEN_AS_NEEDED, weight_time, PARTIAL_RECORD});
  }

  if(mode == Mode::Test) {
    auto X = cxsom::builder::variable(input_prefix + "-in", cxsom::builder::name("X"), "Scalar", 1, FROZEN_TRACE, OPENED);
    auto Y = cxsom::builder::variable(input_prefix + "-in", cxsom::builder::name("Y"), "Scalar", 1, FROZEN_TRACE, OPENED);
    auto U = cxsom::builder::variable(input_prefix + "-in", cxsom::builder::name("U"), "Scalar", 1, FROZEN_TRACE, OPENED);
    X->definition();
    Y->definition();
    U->definition();
  }
  
  if(mode == Mode::Frozen) {
    auto archi = make_architecture(false, UNDEFINED_TRACE);
    auto X = cxsom::builder::variable(input_prefix + "-in", cxsom::builder::name("X"), "Scalar", 1, FROZEN_TRACE, OPENED);
    auto Y = cxsom::builder::variable(input_prefix + "-in", cxsom::builder::name("Y"), "Scalar", 1, FROZEN_TRACE, OPENED);
    auto U = cxsom::builder::variable(input_prefix + "-in", cxsom::builder::name("U"), "Scalar", 1, FROZEN_TRACE, OPENED);
    
    archi->frozen({input_prefix, analysis_prefix, CACHE, FROZEN_TRACE, OPEN_AS_NEEDED, weight_time, PARTIAL_RECORD});
   }

  
  return 0;
}

