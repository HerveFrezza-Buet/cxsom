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
  
  kwd::parameters p_main, p_match, p_learn, p_learn_e, p_learn_c, p_external, p_contextual, p_global;
  p_main       | kwd::use("walltime", WALLTIME), kwd::use("epsilon", 0);
  p_match      | p_main, kwd::use("sigma", .2);
  p_learn      | p_main, kwd::use("alpha", .05);
  p_learn_e    | p_learn, kwd::use("r", .25 );
  p_learn_c    | p_learn, kwd::use("r", .075);
  p_external   | p_main;
  p_contextual | p_main;
  p_global     | p_main, kwd::use("random-bmu", 1), kwd::use("sigma", .01), kwd::use("beta", .5), kwd::use("delta", .01), kwd::use("deadline", 100);

  
  auto X = cxsom::builder::variable("in", cxsom::builder::name("X"), "Scalar", CACHE, TRACE, OPENED);
  auto Y = cxsom::builder::variable("in", cxsom::builder::name("Y"), "Scalar", CACHE, TRACE, OPENED);
  X->definition();
  Y->definition();

  auto Xmap = cxsom::builder::map1D("X", MAP_SIZE, CACHE, TRACE, OPENED);
  *Xmap     = {p_external, p_contextual, p_global};

  auto Ymap = cxsom::builder::map1D("Y", MAP_SIZE, CACHE, TRACE, OPENED);
  *Ymap     = {p_external, p_contextual, p_global};

  auto Amap = cxsom::builder::map1D("Assoc", MAP_SIZE, CACHE, TRACE, OPENED);
  *Amap     = {p_external, p_contextual, p_global};


  Xmap->external  (X,    fx::match_gaussian, p_match, fx::learn_triangle, p_learn_e);
  Xmap->contextual(Amap, fx::match_gaussian, p_match, fx::learn_triangle, p_learn_c);

  Ymap->external  (Y,    fx::match_gaussian, p_match, fx::learn_triangle, p_learn_e);
  Ymap->contextual(Amap, fx::match_gaussian, p_match, fx::learn_triangle, p_learn_c);

  Amap->contextual(Xmap, fx::match_gaussian, p_match, fx::learn_triangle, p_learn_c);
  Amap->contextual(Ymap, fx::match_gaussian, p_match, fx::learn_triangle, p_learn_c);

  
  archi << Xmap << Ymap << Amap;
  archi->realize();
  
  // Updates of weights reads weights at t-1... so timestep 0 connot
  // be implement with the general updating rule. Let us specify it as
  // setting all the variables randomly.
  Xmap->internals_random_at(0);
  Ymap->internals_random_at(0);
  Amap->internals_random_at(0);
  
  
  std::ofstream dot_file("XY-associative-archi.dot");
  dot_file << archi->write_dot;
  
  return 0;
}
