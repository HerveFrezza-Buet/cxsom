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
#define TRAIN_TRACE       10
#define TEST_TRACE     10000
#define OPENED          true
#define OPEN_AS_NEEDED false
#define FORGET             0
#define FOREVER           -1 // Infinite walltime
#define DEADLINE         100

#define MAP_SIZE         1500
#define IMG_SIDE          100

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
  auto W   = cxsom::builder::variable("in", cxsom::builder::name("w"),     "Pos1D", CACHE, TRAIN_TRACE, OPENED);
  auto H   = cxsom::builder::variable("in", cxsom::builder::name("h"),     "Pos1D", CACHE, TRAIN_TRACE, OPENED);
  auto RGB = cxsom::builder::variable("in", cxsom::builder::name("rgb"), "Array=3", CACHE, TRAIN_TRACE, OPENED);
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

#define Rext .02
#define Rctx .002
struct Params {
  kwd::parameters
    main,
    match_pos, match_rgb,
    learn,
    learn_pos_e, learn_pos_c,
    learn_rgb_e, learn_rgb_c,
    external, contextual, global;
  Params() {
    main        | kwd::use("walltime", FOREVER), kwd::use("epsilon", 0);
    match_pos   | main,  kwd::use("sigma", .2);
    match_rgb   | main,  kwd::use("sigma", .2);
    learn       | main,  kwd::use("alpha", .1);
    learn_pos_e | learn, kwd::use("r", Rext);
    learn_pos_c | learn, kwd::use("r", Rctx);
    learn_rgb_e | learn, kwd::use("r", Rext);
    learn_rgb_c | learn, kwd::use("r", Rctx);
    external    | main;
    contextual  | main;
    global      | main,  kwd::use("random-bmu", 1), kwd::use("beta", .5), kwd::use("delta", .02), kwd::use("deadline", DEADLINE);
  }
};

auto make_map_settings(const Params& p) {
  auto map_settings = cxsom::builder::map::make_settings();
  map_settings.map_size          = MAP_SIZE;
  map_settings.cache_size        = CACHE;
  map_settings.weights_file_size = TRAIN_TRACE;
  map_settings.kept_opened       = OPENED;
  map_settings                   = {p.external, p.contextual, p.global};
  map_settings.argmax            = fx::argmax;
  map_settings.toward_argmax     = fx::toward_argmax;

  return map_settings;
}


void make_train_rules(unsigned int save_period) {
  

  Params p;
  auto map_settings = make_map_settings(p);
  
  auto archi = cxsom::builder::architecture();
  archi->timelines = {"train-wgt", "train-rlx", "train-out"};

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
  std::vector<cxsom::builder::Map::Layer*> layers;
  auto out_layer = std::back_inserter(layers);
  
  // Let us define the maps
  auto Wmap = cxsom::builder::map::make_1D("W"  );
  auto Hmap = cxsom::builder::map::make_1D("H"  );  
  auto Rmap = cxsom::builder::map::make_1D("RGB");

  // We store the layers since we have to add rules for saving weights.
  // Let us connect the maps together
  *(out_layer++) = Wmap->contextual(Hmap, fx::match_gaussian, p.match_pos, fx::learn_triangle, p.learn_pos_c);
  *(out_layer++) = Wmap->contextual(Rmap, fx::match_gaussian, p.match_pos, fx::learn_triangle, p.learn_pos_c);
  *(out_layer++) = Hmap->contextual(Wmap, fx::match_gaussian, p.match_pos, fx::learn_triangle, p.learn_pos_c);
  *(out_layer++) = Hmap->contextual(Rmap, fx::match_gaussian, p.match_pos, fx::learn_triangle, p.learn_pos_c);
  *(out_layer++) = Rmap->contextual(Wmap, fx::match_gaussian, p.match_pos, fx::learn_triangle, p.learn_rgb_c);
  *(out_layer++) = Rmap->contextual(Hmap, fx::match_gaussian, p.match_pos, fx::learn_triangle, p.learn_rgb_c);

  // Let us provide inputs to the maps.
  *(out_layer++) = Wmap->external(W, fx::match_gaussian, p.match_pos, fx::learn_triangle, p.learn_pos_e);
  *(out_layer++) = Hmap->external(H, fx::match_gaussian, p.match_pos, fx::learn_triangle, p.learn_pos_e);
  *(out_layer++) = Rmap->external(R, fx::match_gaussian, p.match_rgb, fx::learn_triangle, p.learn_rgb_e);

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

  // We add the rules for weight savings.
  for(auto layer_ptr : layers) {
    auto W = layer_ptr->_W();
    auto Wsaved = cxsom::builder::variable("saved", W->varname, W->type, CACHE, SAVE_TRACE, OPEN_AS_NEEDED);
    Wsaved->definition();
    Wsaved->var() << fx::copy(kwd::times(W->var(), save_period)) | kwd::use("walltime", FOREVER);
  }
}


void make_test_rules(unsigned int saved_weight_at) {
  Params p;
  auto map_settings = make_map_settings(p);
  
  auto archi = cxsom::builder::architecture();
  archi->timelines = {"test-wgt", "test-rlx", "test-out"};

  // Let us retrieve the feature of the weight variables (only for
  // those who host Pos1D prototypes here).
  std::string wtype;
  {
    std::ostringstream ostr;
    ostr << "Map1D<Pos1D>=" << MAP_SIZE;
    wtype = ostr.str();
  }
  auto cache = *(map_settings.cache_size);
  auto trace = *(map_settings.weights_file_size);
  auto kopen = *(map_settings.kept_opened);
  
  // Let us define the maps
  auto Wmap = cxsom::builder::map::make_1D("W"  );
  auto Hmap = cxsom::builder::map::make_1D("H"  );  
  auto Rmap = cxsom::builder::map::make_1D("RGB");

  // Let us connect the map, using non-adaptive layers and the weights
  // learnt previously, instead of internally defined weights.
  auto Wc0 = cxsom::builder::variable("saved", cxsom::builder::name("W")   / cxsom::builder::name("Wc-0"), wtype, cache, trace, kopen);
  auto Wc1 = cxsom::builder::variable("saved", cxsom::builder::name("W")   / cxsom::builder::name("Wc-1"), wtype, cache, trace, kopen);
  auto Hc0 = cxsom::builder::variable("saved", cxsom::builder::name("H")   / cxsom::builder::name("Wc-0"), wtype, cache, trace, kopen);
  auto Hc1 = cxsom::builder::variable("saved", cxsom::builder::name("H")   / cxsom::builder::name("Wc-1"), wtype, cache, trace, kopen);
  auto Rc0 = cxsom::builder::variable("saved", cxsom::builder::name("RGB") / cxsom::builder::name("Wc-0"), wtype, cache, trace, kopen);
  auto Rc1 = cxsom::builder::variable("saved", cxsom::builder::name("RGB") / cxsom::builder::name("Wc-1"), wtype, cache, trace, kopen);
  Wmap->contextual(Hmap, fx::match_gaussian, p.match_pos, Wc0, saved_weight_at);
  Wmap->contextual(Rmap, fx::match_gaussian, p.match_pos, Wc1, saved_weight_at);
  Hmap->contextual(Wmap, fx::match_gaussian, p.match_pos, Hc0, saved_weight_at);
  Hmap->contextual(Rmap, fx::match_gaussian, p.match_pos, Hc1, saved_weight_at);
  Rmap->contextual(Wmap, fx::match_gaussian, p.match_pos, Rc0, saved_weight_at);
  Rmap->contextual(Hmap, fx::match_gaussian, p.match_pos, Rc1, saved_weight_at);

  // Let us declare the inputs (W, H) and the output (RGB).
  auto W   = cxsom::builder::variable("prediction-input" , cxsom::builder::name("w")  , "Pos1D"  , CACHE, TEST_TRACE, OPENED);
  auto H   = cxsom::builder::variable("prediction-input" , cxsom::builder::name("h")  , "Pos1D"  , CACHE, TEST_TRACE, OPENED);
  auto RGB = cxsom::builder::variable("prediction-output", cxsom::builder::name("rgb"), "Array=3", CACHE, TEST_TRACE, OPENED);
  W->definition();
  H->definition();
  RGB->definition();

  // Let us provide inputs to W, and H maps, using weights learnt previously as well.
  auto We0 = cxsom::builder::variable("saved", cxsom::builder::name("W") / cxsom::builder::name("We-0"), wtype, cache, trace, kopen);
  auto He0 = cxsom::builder::variable("saved", cxsom::builder::name("H") / cxsom::builder::name("We-0"), wtype, cache, trace, kopen);  
  Wmap->external(W, fx::match_gaussian, p.match_pos, We0, saved_weight_at);
  Hmap->external(H, fx::match_gaussian, p.match_pos, He0, saved_weight_at);

  // We will need the external RGB weights for retreiving the RGB value.
  {
    std::ostringstream ostr;
    ostr << "Map1D<Array=3>=" << MAP_SIZE;
    wtype = ostr.str();
  }
  auto Re0 = cxsom::builder::variable("saved", cxsom::builder::name("RGB") / cxsom::builder::name("We-0"), wtype, cache, trace, kopen);

 

  // We define all the external weights, for the sake of comprehensive
  // graphs when the architecture is displayed.
  We0->definition();
  Wc0->definition();
  Wc1->definition();
  He0->definition();
  Hc0->definition();
  Hc1->definition();
  Re0->definition();
  Rc0->definition();
  Rc1->definition();

  // Let us build up and configure the architecture
  archi << Wmap << Hmap << Rmap;
  *archi = map_settings;
  
  // We can now produce the cxsom rules and the description graph.
  archi->realize();
  {
    std::ofstream dot_file("test.dot");
    dot_file << archi->write_dot;
    std::cout << "File \"test.dot\" generated." << std::endl;
  }

  // Now, we need supplementary rules for reading the RGB
  // result. Indeed, it consists in usingr the BMU of the RGB map to
  // index the learnt RGB weights.
  RGB->var() << fx::value_at(kwd::at(Re0->var(), saved_weight_at), Rmap->output_BMU()->var()) | kwd::use("walltime", FOREVER   );
}

int main(int argc, char* argv[]) {
  context c(argc, argv);
  Mode mode;
  unsigned int walltime        = 0;
  unsigned int save_period     = 0;
  unsigned int saved_weight_at = 0;
  
  // We analyse the arguments and identify the mode.
  std::ostringstream prefix;
  for(const auto& arg : c.argv) prefix << arg << ' ';
  prefix << "-- ";
  
  if(c.user_argv.size() == 0) {
    std::cout << "You have to provide user arguments." << std::endl
	      << "e.g:" << std::endl
	      << "  " << prefix.str() << "input                  <-- sends the rules for the inputs." << std::endl
	      << "  " << prefix.str() << "walltime <max-time>    <-- sends the rules for the inputs wall-time redefinition." << std::endl
	      << "  " << prefix.str() << "train <save-period>    <-- sends the rules for training." << std::endl
	      << "  " << prefix.str() << "test <saved-weight-at> <-- sends the rules for testing." << std::endl;
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
  else if(c.user_argv[0] == "test") {
    if(c.user_argv.size() != 2) {
      std::cout << "The 'test' mode expects a saved-weight-at argument"  << std::endl;
      c.notify_user_argv_error(); 
      return 0;
    }
    saved_weight_at = stoul(c.user_argv[1]);
    mode = Mode::Test;
  }
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
    break;
  case Mode::Test:
    make_test_rules(saved_weight_at);
    break;
  default:
    break;
  }
  
  return 0;
}
