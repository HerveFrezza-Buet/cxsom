/*

  Here, we show how to expand the rules in order to exhibit the
  relaxation process. We consider 2 connexted maps with external
  inputs.

  See the assorted stuff in the 'experiment' section.

*/

#include <cxsom-builder.hpp>
#include <fstream>
#include <sstream>


#define CACHE       2
#define TRACE   10000
#define OPENED   true
#define FORGET      0
#define WALLTIME   -1 // Infinite walltime
#define DEADLINE  200

#define MAP_SIZE 500

// cxsom declarations
using namespace cxsom::rules;
context* cxsom::rules::ctx = nullptr;

auto make_architecture(bool define_inputs) {

  auto archi = cxsom::builder::architecture();
  
  kwd::parameters p_main, p_match, p_learn, p_learn_e, p_learn_c, p_external, p_contextual, p_global;
  p_main       | kwd::use("walltime", WALLTIME), kwd::use("epsilon", 0);
  p_match      | p_main, kwd::use("sigma", .2);
  p_learn      | p_main, kwd::use("alpha", .05);
  p_learn_e    | p_learn, kwd::use("r", .25 );
  p_learn_c    | p_learn, kwd::use("r", .075);
  p_external   | p_main;
  p_contextual | p_main;
  p_global     | p_main, kwd::use("random-bmu", 1), kwd::use("beta", .5), kwd::use("delta", .01), kwd::use("deadline", DEADLINE);

  auto X = cxsom::builder::variable("in", cxsom::builder::name("X"), "Scalar", CACHE, TRACE, OPENED);
  auto Y = cxsom::builder::variable("in", cxsom::builder::name("Y"), "Scalar", CACHE, TRACE, OPENED);
  if(define_inputs) {
    X->definition();
    Y->definition();
  }
  
  auto Xmap = cxsom::builder::map1D("X", MAP_SIZE, CACHE, TRACE, OPENED);
  auto Ymap = cxsom::builder::map1D("Y", MAP_SIZE, CACHE, TRACE, OPENED);
  *Xmap     = {p_external, p_contextual, p_global};
  *Ymap     = {p_external, p_contextual, p_global};
  
  Xmap->external  (X,    fx::match_gaussian, p_match, fx::learn_triangle, p_learn_e);
  Xmap->contextual(Ymap, fx::match_gaussian, p_match, fx::learn_triangle, p_learn_c);
  Ymap->external  (Y,    fx::match_gaussian, p_match, fx::learn_triangle, p_learn_e);
  Ymap->contextual(Xmap, fx::match_gaussian, p_match, fx::learn_triangle, p_learn_c);

  archi << Xmap << Ymap;
  
  archi->relax_count = "Cvg"; 
  return archi;
}

enum class Mode : char {Main, Relax};

int main(int argc, char* argv[]) {
  context c(argc, argv);
  Mode mode = Mode::Main;
  std::string expand_prefix;
  unsigned int weight_time;


  // We analyse the arguments and identify the mode.
  std::ostringstream prefix;
  for(const auto& arg : c.argv) prefix << arg << ' ';
  prefix << "-- ";
    
  if(c.user_argv.size() == 0) {
    std::cout << "You have to provide user arguments." << std::endl
	      << "e.g:" << std::endl
	      << "  " << prefix.str() << "main                            <-- sends the main rules." << std::endl
	      << "  " << prefix.str() << "relax <timeline-prefix> <time>  <-- sends relaxation rules for weights at time." << std::endl;
    c.notify_user_argv_error(); 
    return 0;
  }

  if(c.user_argv[0] == "main")
    mode = Mode::Main;
  else if(c.user_argv[0] == "relax") {
    if(c.user_argv.size() != 3) {
      std::cout << "The 'relax' mode expects 2 arguments"  << std::endl;
      c.notify_user_argv_error(); 
      return 0;
    }
    mode = Mode::Relax;
    expand_prefix = c.user_argv[1];
    weight_time = stoul(c.user_argv[2]);
  }
  else {
    std::cout << "Bad user arguments." << std::endl;
    c.notify_user_argv_error(); 
    return 0;
  }
  

  // Now, according to the mode, let us send rules.

  if(mode == Mode::Main) {
    auto archi = make_architecture(true);
    archi->realize();
    for(auto map : archi->maps) map->internals_random_at(0);

    if(c.argv[1] == "graph" || c.argv[1] == "graph-full") {
      std::ofstream dot_file("archi.dot");
      dot_file << archi->write_dot;
    }
  }

  
  if(mode == Mode::Relax) {
    auto archi = make_architecture(false);
    auto X = cxsom::builder::variable(expand_prefix + "-in", cxsom::builder::name("X"), "Scalar", CACHE, TRACE, OPENED);
    auto Y = cxsom::builder::variable(expand_prefix + "-in", cxsom::builder::name("Y"), "Scalar", CACHE, TRACE, OPENED);
    X->definition();
    Y->definition();
    
    archi->expand_relax({expand_prefix, CACHE, TRACE, OPENED, weight_time});
  }

  
  return 0;
}

