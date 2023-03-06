/*

  Here, we consider 3 connected maps one handling W, the other H (the
  coordinates in an image), and the last the RGB value of the
  pixel. We train the map with (W, H, RGB) triplets, and then we let
  the architecture retrieve thr RGB value from the coordinates.

  This serves as a rule generator for the experiment 003-002 in the
  experimental section.
*/

#include <cxsom-builder.hpp>
#include <fstream>
#include <sstream>
#include <tuple>
#include <iterator>

#define CACHE              2
#define SAVE_TRACE      1000
#define TRAIN_TRACE    10000
#define INPUT_TRACE     1000
#define OPENED          true
#define OPEN_AS_NEEDED false
#define FORGET             0
#define FOREVER           -1 // Infinite walltime
#define DEADLINE         100

#define MAP_SIZE         500
#define IMG_SIDE         100

// cxsom declarations
using namespace cxsom::rules;
context* cxsom::rules::ctx = nullptr;

enum class Mode : char {Train, Test, Input, Walltime};

auto img_type() {
  std::ostringstream ostr;
  ostr << "Map2D<Array=3>=" << IMG_SIDE;
  return ostr.str();
}

auto rgb_inputs() {
  auto W   = cxsom::builder::variable("in", cxsom::builder::name("w"),     "Pos1D", CACHE, INPUT_TRACE, OPENED);
  auto H   = cxsom::builder::variable("in", cxsom::builder::name("h"),     "Pos1D", CACHE, INPUT_TRACE, OPENED);
  auto RGB = cxsom::builder::variable("in", cxsom::builder::name("rgb"), "Array=3", CACHE, INPUT_TRACE, OPENED);
  return std::make_tuple(W, H, RGB);
}

void make_walltime_rules(unsigned int walltime) {
  {
    timeline t{"img"};
    "coord" << fx::random() | kwd::use("walltime", walltime);
  }
}

void make_input_rules() {
  auto SRC = cxsom::builder::variable("img", cxsom::builder::name("src"),     img_type(),     1,     1, OPENED);
  auto COORD = cxsom::builder::variable("img", cxsom::builder::name("coord"),    "Pos2D", CACHE,     1, OPENED);
  auto [W, H, RGB] = rgb_inputs();
  SRC->definition();
  COORD->definition();
  W->definition();
  H->definition();
  RGB->definition();

  auto src   = SRC->varname.value;
  auto coord = COORD->varname.value;
  auto w     = W->varname.value;
  auto h     = H->varname.value;
  auto rgb   = RGB->varname.value;

  make_walltime_rules(0);
  
  {
    timeline t{"in"};
    w   << fx::first (kwd::var("img", coord))                                     | kwd::use("walltime", FOREVER);
    h   << fx::second(kwd::var("img", coord))                                     | kwd::use("walltime", FOREVER);
    rgb << fx::value_at(kwd::at(kwd::var("img", src), 0), kwd::var("img", coord)) | kwd::use("walltime", FOREVER);
  }
}

void make_train_rules(unsigned int save_period) {
  
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
  map_settings.weights_file_size = TRAIN_TRACE;
  map_settings.kept_opened       = OPENED;
  map_settings                   = {p_external, p_contextual, p_global};
  map_settings.argmax            = fx::argmax;
  map_settings.toward_argmax     = fx::toward_argmax;

  // Let us declare the inputs
  auto W = cxsom::builder::variable("in", cxsom::builder::name("w")  , "Pos1D" , CACHE, TRAIN_TRACE, OPENED);
  auto H = cxsom::builder::variable("in", cxsom::builder::name("h")  , "Pos1D" , CACHE, TRAIN_TRACE, OPENED);
  auto R = cxsom::builder::variable("in", cxsom::builder::name("rgb"), "Array=3", CACHE, TRAIN_TRACE, OPENED);

  // Sending this definition enables the server to perform type
  // checking.
  W->definition();
  H->definition();
  R->definition();

  // Let us define the maps
  auto Wmap = cxsom::builder::map::make_1D("W"  );
  auto Hmap = cxsom::builder::map::make_1D("H"  );  
  auto Rmap = cxsom::builder::map::make_1D("RGB");

  // We store the layers since we have to add rules for saving waights.
  std::vector<cxsom::builder::Map::Layer*> layers;
  auto out_layer = std::back_inserter(layers);
  // Let us provide inputs to the maps.
  *(out_layer++) = Wmap->external(W, fx::match_gaussian, p_match, fx::learn_triangle, p_learn_e);
  *(out_layer++) = Hmap->external(H, fx::match_gaussian, p_match, fx::learn_triangle, p_learn_e);
  *(out_layer++) = Rmap->external(R, fx::match_gaussian, p_match, fx::learn_triangle, p_learn_e);

  // Let us connect the maps together
  *(out_layer++) = Wmap->contextual(Hmap, fx::match_gaussian, p_match, fx::learn_triangle, p_learn_c);
  *(out_layer++) = Wmap->contextual(Rmap, fx::match_gaussian, p_match, fx::learn_triangle, p_learn_c);
  *(out_layer++) = Hmap->contextual(Wmap, fx::match_gaussian, p_match, fx::learn_triangle, p_learn_c);
  *(out_layer++) = Hmap->contextual(Rmap, fx::match_gaussian, p_match, fx::learn_triangle, p_learn_c);
  *(out_layer++) = Rmap->contextual(Wmap, fx::match_gaussian, p_match, fx::learn_triangle, p_learn_c);
  *(out_layer++) = Rmap->contextual(Hmap, fx::match_gaussian, p_match, fx::learn_triangle, p_learn_c);

  // Let us build up and configure the architecture
  archi << Wmap << Hmap << Rmap;
  *archi = map_settings;

  // We set random values for weights at first timestep. This avoid
  // blocking at step 0 since learning rules consider the weights at
  // previous timestep.
  for(auto map : archi->maps) map->internals_random_at(0);

  // We can now produce the cxsom rules and the description graph.
  archi->realize();
  {
    std::ofstream dot_file("train.dot");
    dot_file << archi->write_dot;
    std::cout << "File \"train.dot\" generated." << std::endl;
  }

  // Let us add weight saving rules.
  for(auto layer_ptr : layers) {
    auto W = layer_ptr->_W();
    auto Wsaved = cxsom::builder::variable("saved", W->varname, W->type, CACHE, SAVE_TRACE, OPEN_AS_NEEDED);
    Wsaved->definition();
    Wsaved->var() << fx::copy(kwd::times(W->var(), save_period)) | kwd::use("walltime", FOREVER);
  }
}


int main(int argc, char* argv[]) {
  context c(argc, argv);
  Mode mode;
  unsigned int walltime = 0;
  unsigned int save_period = 0;
  
  // We analyse the arguments and identify the mode.
  std::ostringstream prefix;
  for(const auto& arg : c.argv) prefix << arg << ' ';
  prefix << "-- ";
  
  if(c.user_argv.size() == 0) {
    std::cout << "You have to provide user arguments." << std::endl
	      << "e.g:" << std::endl
	      << "  " << prefix.str() << "input               <-- sends the rules for the inputs." << std::endl
	      << "  " << prefix.str() << "walltime <max-time> <-- sends the rules for the inputs wall-time redefinition." << std::endl
	      << "  " << prefix.str() << "train <save-period> <-- sends the rules for training." << std::endl
	      << "  " << prefix.str() << "test                <-- sends the rules for testing." << std::endl;
    c.notify_user_argv_error(); 
    return 0;
  }

  if(c.user_argv[0] == "walltime") {
    if(c.user_argv.size() != 2) {
      std::cout << "The 'input' mode expects a max-time argument"  << std::endl;
      c.notify_user_argv_error(); 
      return 0;
    }
    walltime = stoul(c.user_argv[1]);
    mode = Mode::Walltime;
  }
  else if(c.user_argv[0] == "input")
    mode = Mode::Input;
  else if(c.user_argv[0] == "train") {
    if(c.user_argv.size() != 2) {
      std::cout << "The 'train' mode expects a save-period argument"  << std::endl;
      c.notify_user_argv_error(); 
      return 0;
    }
    save_period = stoul(c.user_argv[1]);
    mode = Mode::Train;
  }
  else if(c.user_argv[0] == "test")
    mode = Mode::Test;
  else {
    std::cout << "Bad user arguments." << std::endl;
    c.notify_user_argv_error(); 
    return 0;
  }

  switch(mode) {
  case Mode::Input:
    make_input_rules();
    break;
  case Mode::Walltime:
    make_walltime_rules(walltime);
    break;
  case Mode::Train:
    make_train_rules(save_period);
  case Mode::Test:
  default:
    break;
  }
  
  return 0;
}
