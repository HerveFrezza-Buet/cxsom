/*

  Here, we consider 3 connected maps one handling W, the other H (the
  coordinates in an image), and the last the RGB value of the
  pixel. We train the map with (W, H, RGB) triplets, and then we let
  the architecture retrieve thr RGB value from the coordinates.

  There ar several modes, each one is made of specific rules. See make_*_rule functions.
*/

#include <cxsom-builder.hpp>
#include <fstream>
#include <sstream>
#include <tuple>
#include <iterator>

#define CACHE              2
#define SAVE_TRACE      1000
#define TRAIN_TRACE       10
#define OPENED          true
#define OPEN_AS_NEEDED false
#define FORGET             0
#define FOREVER           -1 // Infinite walltime
#define DEADLINE         100

#define MAP_SIZE         1500

// cxsom declarations
using namespace cxsom::rules;
context* cxsom::rules::ctx = nullptr;

enum class Mode : char {Calibration, Input, Train, Check, Predict, Walltime};

// ####################
// #                  #
// # General settings #
// #                  #
// ####################

#define Rext .05
#define Rctx .003
struct Params {
  kwd::parameters
    main,
    match_ctx, match_pos, match_rgb,
    learn,
    learn_pos_e, learn_pos_c,
    learn_rgb_e, learn_rgb_c,
    external, contextual, global;
  Params() {
    main        | kwd::use("walltime", FOREVER), kwd::use("epsilon", 0);
    match_ctx   | main,  kwd::use("sigma", .075);
    match_pos   | main,  kwd::use("sigma", .075);
    match_rgb   | main,  kwd::use("sigma", .075);
    learn       | main,  kwd::use("alpha", .1   );
    learn_pos_e | learn, kwd::use("r"    , Rext);
    learn_pos_c | learn, kwd::use("r"    , Rctx);
    learn_rgb_e | learn, kwd::use("r"    , Rext);
    learn_rgb_c | learn, kwd::use("r"    , Rctx);
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


auto rgb_inputs(const std::string& timeline, unsigned int trace, bool to_be_defined) {
  auto W   = cxsom::builder::variable(timeline, cxsom::builder::name("w"),     "Pos1D", CACHE, trace, OPENED);
  auto H   = cxsom::builder::variable(timeline, cxsom::builder::name("h"),     "Pos1D", CACHE, trace, OPENED);
  auto RGB = cxsom::builder::variable(timeline, cxsom::builder::name("rgb"), "Array=3", CACHE, trace, OPENED);
  if(to_be_defined) {
    W->definition();
    H->definition();
    RGB->definition();
  }
  return std::make_tuple(W, H, RGB);
}

// #####################
// #                   #
// # Calibration stage #
// #                   #
// #####################


void make_calibration_rules(unsigned grid_side) {
  Params p;

  // This mode is not really a simulation, but a mode where the impact
  // of the Gaussian parameter values can be displayed, in order to
  // tune them (i.e. calibration).

  {
    timeline t("calibration");

    
    kwd::type("rgb-ref"    , "Array=3",                                                              2, 1, OPENED);
    kwd::type("pos-ref"    , "Scalar"                                                              , 2, 1, OPENED);
    kwd::type("rgb-samples", std::string("Map1D<Array=3>=") + std::to_string(grid_side * grid_side), 2, 1, OPENED);
    kwd::type("pos-samples", std::string("Map1D<Scalar>=")  + std::to_string(grid_side)            , 2, 1, OPENED);
    kwd::type("rgb-match"  , std::string("Map1D<Scalar>=")  + std::to_string(grid_side * grid_side), 2, 1, OPENED); 
    kwd::type("pos-match"  , std::string("Map1D<Scalar>=")  + std::to_string(grid_side)            , 2, 1, OPENED);
    kwd::type("ctx-match"  , std::string("Map1D<Scalar>=")  + std::to_string(grid_side)            , 2, 1, OPENED);

    kwd::at("rgb-match", 0) << fx::match_gaussian(kwd::at("rgb-ref", 0), kwd::at("rgb-samples", 0)) | p.match_rgb;
    kwd::at("pos-match", 0) << fx::match_gaussian(kwd::at("pos-ref", 0), kwd::at("pos-samples", 0)) | p.match_pos;
    kwd::at("ctx-match", 0) << fx::match_gaussian(kwd::at("pos-ref", 0), kwd::at("pos-samples", 0)) | p.match_ctx; 
  }
  
}

// ##################
// #                #
// # Walltime stage #
// #                #
// ##################

void make_walltime_rules(unsigned int walltime) {
  {
    timeline t{"train-in"};
    "coord" << fx::random() | kwd::use("walltime", walltime);
  }
}


// ###############
// #             #
// # Input stage #
// #             #
// ###############

void make_input_rules(unsigned int img_side) {
  auto [W, H, RGB] = rgb_inputs("img", img_side * img_side, true);
  auto COORD = cxsom::builder::variable("img", cxsom::builder::name("coord"), "Pos2D"                                                  , CACHE,  1, OPEN_AS_NEEDED);
  auto SRC   = cxsom::builder::variable("img", cxsom::builder::name("src")  , std::string("Map2D<Array=3>=") + std::to_string(img_side),     1,  1, OPEN_AS_NEEDED);
  COORD->definition();
  SRC->definition();
  
  COORD->var() << fx::pair(W->var(), H->var())                       | kwd::use("walltime", FOREVER);
  RGB->var()   << fx::value_at(kwd::at(SRC->var(), 0), COORD->var()) | kwd::use("walltime", FOREVER);
  
}

// ###############
// #             #
// # Train stage #
// #             #
// ###############

void make_train_rules(unsigned int save_period, unsigned int img_side) {
  
  Params p;
  auto map_settings = make_map_settings(p);
  
  auto archi = cxsom::builder::architecture();
  archi->timelines = {"train-wgt", "train-rlx", "train-out"};

  // These are the inputs variables definitions. The rules for
  // computing them are provided below.
  auto [W, H, R] = rgb_inputs("train-in", TRAIN_TRACE, true);
  auto COORD = cxsom::builder::variable("train-in", cxsom::builder::name("coord"), "Pos2D", CACHE, TRAIN_TRACE, OPENED);
  COORD->definition();

  // Let us define the maps
  
  std::vector<cxsom::builder::Map::Layer*> layers; // This will store all layers 
  auto out_layer = std::back_inserter(layers);     // for further manipulation on them.
  
  // Let us define the maps
  auto Wmap = cxsom::builder::map::make_1D("W"  );
  auto Hmap = cxsom::builder::map::make_1D("H"  );  
  auto Rmap = cxsom::builder::map::make_1D("RGB");

  // We store the layers since we have to add rules for saving weights.
  // Let us connect the maps together

  // First, we set up all the "contextual" (i.e. from map to map) connections.
  *(out_layer++) = Wmap->contextual(Hmap, fx::match_gaussian, p.match_ctx, fx::learn_triangle, p.learn_pos_c);
  *(out_layer++) = Wmap->contextual(Rmap, fx::match_gaussian, p.match_ctx, fx::learn_triangle, p.learn_pos_c);
  *(out_layer++) = Hmap->contextual(Wmap, fx::match_gaussian, p.match_ctx, fx::learn_triangle, p.learn_pos_c);
  *(out_layer++) = Hmap->contextual(Rmap, fx::match_gaussian, p.match_ctx, fx::learn_triangle, p.learn_pos_c);
  *(out_layer++) = Rmap->contextual(Wmap, fx::match_gaussian, p.match_ctx, fx::learn_triangle, p.learn_rgb_c);
  *(out_layer++) = Rmap->contextual(Hmap, fx::match_gaussian, p.match_ctx, fx::learn_triangle, p.learn_rgb_c);

  // Let us provide inputs to the maps, thanks to the "external" layers.
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
  }

  // We add the rules for the computation of inputs.
  auto SRC = cxsom::builder::variable("img", cxsom::builder::name("src"), std::string("Map2D<Array=3>=") + std::to_string(img_side), 1, 1, OPEN_AS_NEEDED);
  SRC->definition(); // avoids ??? in architecture display.
  COORD->var() << fx::random()                                       | kwd::use("walltime", 0);   // This wil be changed by clients, for actually triggering new inputs.
  W->var()     << fx::first(COORD->var())                            | kwd::use("walltime", FOREVER);
  H->var()     << fx::second(COORD->var())                           | kwd::use("walltime", FOREVER);
  R->var()     << fx::value_at(kwd::at(SRC->var(), 0), COORD->var()) | kwd::use("walltime", FOREVER); // We get the rgb value from the image collected at "Input" stage.
  
  
  // We add the rules for weight savings. It consists of saving
  // weights externally (i.e. the 'saved' timeline) every save_period
  // time steps.
  for(auto layer_ptr : layers) {
    auto W = layer_ptr->_W();
    auto Wsaved = cxsom::builder::variable("saved", W->varname, W->type, CACHE, SAVE_TRACE, OPEN_AS_NEEDED);
    Wsaved->definition();
    Wsaved->var() << fx::copy(kwd::times(W->var(), save_period)) | kwd::use("walltime", FOREVER);
  }
}



// ###############
// #             #
// # Check stage #
// #             #
// ###############

void make_check_rules(unsigned int saved_weight_at, unsigned int img_side) {
  
  unsigned int trace = img_side * img_side;
  Params p;
  auto map_settings = make_map_settings(p);
  map_settings.exposure_file_size = trace; // At the end of this function, we exposed W, H and RGB weights, we keep trace of them as for a (w, h, rgb) dataset.

  // We could have recordered the BMU positions of the maps like this.
  // map_settings.bmu_file_size = trace;
  
  auto archi = cxsom::builder::architecture();
  archi->timelines = {"check-wgt", "check-rlx", "check-out"};

  // Let us retrieve the feature of the weight variables (only for
  // those who host Pos1D prototypes here).
  std::string  wtype = std::string("Map1D<Pos1D>="  ) + std::to_string(MAP_SIZE);
  std::string  rtype = std::string("Map1D<Array=3>=") + std::to_string(MAP_SIZE);
  
  // Let us define the maps
  auto Wmap = cxsom::builder::map::make_1D("W"  );
  auto Hmap = cxsom::builder::map::make_1D("H"  );  
  auto Rmap = cxsom::builder::map::make_1D("RGB");

  // Let us connect the map, using **non-adaptive** layers and the weights
  // learnt previously, instead of internally defined weights.
  auto Wc0 = cxsom::builder::variable("saved", cxsom::builder::name("W")   / cxsom::builder::name("Wc-0"), wtype, CACHE, trace, OPENED);
  auto Wc1 = cxsom::builder::variable("saved", cxsom::builder::name("W")   / cxsom::builder::name("Wc-1"), wtype, CACHE, trace, OPENED);
  auto Hc0 = cxsom::builder::variable("saved", cxsom::builder::name("H")   / cxsom::builder::name("Wc-0"), wtype, CACHE, trace, OPENED);
  auto Hc1 = cxsom::builder::variable("saved", cxsom::builder::name("H")   / cxsom::builder::name("Wc-1"), wtype, CACHE, trace, OPENED);
  auto Rc0 = cxsom::builder::variable("saved", cxsom::builder::name("RGB") / cxsom::builder::name("Wc-0"), wtype, CACHE, trace, OPENED);
  auto Rc1 = cxsom::builder::variable("saved", cxsom::builder::name("RGB") / cxsom::builder::name("Wc-1"), wtype, CACHE, trace, OPENED);
  
  // These calls build up non adaptive layers, using external weights.
  Wmap->contextual(Hmap, fx::match_gaussian, p.match_ctx, Wc0, saved_weight_at);
  Wmap->contextual(Rmap, fx::match_gaussian, p.match_ctx, Wc1, saved_weight_at);
  Hmap->contextual(Wmap, fx::match_gaussian, p.match_ctx, Hc0, saved_weight_at);
  Hmap->contextual(Rmap, fx::match_gaussian, p.match_ctx, Hc1, saved_weight_at);
  Rmap->contextual(Wmap, fx::match_gaussian, p.match_ctx, Rc0, saved_weight_at);
  Rmap->contextual(Hmap, fx::match_gaussian, p.match_ctx, Rc1, saved_weight_at);

  // Let us declare the inputs (W, H, RGB) .
  auto W   = cxsom::builder::variable("img", cxsom::builder::name("w")  , "Pos1D"  , CACHE, trace, OPENED);
  auto H   = cxsom::builder::variable("img", cxsom::builder::name("h")  , "Pos1D"  , CACHE, trace, OPENED);
  auto RGB = cxsom::builder::variable("img", cxsom::builder::name("rgb"), "Array=3", CACHE, trace, OPENED);
  W->definition();
  H->definition();
  RGB->definition();

  // Let us provide inputs to W, H and RGB maps, using weights learnt previously as well.
  auto We0 = cxsom::builder::variable("saved", cxsom::builder::name("W")   / cxsom::builder::name("We-0"), wtype, CACHE, trace, OPENED);
  auto He0 = cxsom::builder::variable("saved", cxsom::builder::name("H")   / cxsom::builder::name("We-0"), wtype, CACHE, trace, OPENED);  
  auto Re0 = cxsom::builder::variable("saved", cxsom::builder::name("RGB") / cxsom::builder::name("We-0"), rtype, CACHE, trace, OPENED);
  
  // We expose the value of the external weights at each BMU... This
  // is the checking. This has to be close to the input. Using the
  // cxsom::builder::expose::weight tag for a layer means that we have
  // a variable computing weight(bmu) for that layer.
  Wmap->external(W  , fx::match_gaussian, p.match_pos, We0, saved_weight_at) | cxsom::builder::expose::weight;
  Hmap->external(H  , fx::match_gaussian, p.match_pos, He0, saved_weight_at) | cxsom::builder::expose::weight;
  Rmap->external(RGB, fx::match_gaussian, p.match_rgb, Re0, saved_weight_at) | cxsom::builder::expose::weight;

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
    std::ofstream dot_file("check.dot");
    dot_file << archi->write_dot;
  }
}


// #################
// #               #
// # Predict stage #
// #               #
// #################

void make_predict_rules(unsigned int saved_weight_at, unsigned int img_side) {
  
  Params p;
  auto map_settings = make_map_settings(p);
  
  auto archi = cxsom::builder::architecture();
  archi->timelines = {"predict-wgt", "predict-rlx", "predict-out"};

  // Let us retrieve the feature of the weight variables (only for
  // those who host Pos1D prototypes here).
  std::string  wtype = std::string("Map1D<Pos1D>="  ) + std::to_string(MAP_SIZE);
  std::string  rtype = std::string("Map1D<Array=3>=") + std::to_string(MAP_SIZE);
  unsigned int trace = img_side * img_side;
  
  // Let us define the maps
  auto Wmap = cxsom::builder::map::make_1D("W"  );
  auto Hmap = cxsom::builder::map::make_1D("H"  );  
  auto Rmap = cxsom::builder::map::make_1D("RGB");

  // Let us connect the map, using non-adaptive layers and the weights
  // learnt previously, instead of internally defined weights.
  auto Wc0 = cxsom::builder::variable("saved", cxsom::builder::name("W")   / cxsom::builder::name("Wc-0"), wtype, CACHE, trace, OPENED);
  auto Wc1 = cxsom::builder::variable("saved", cxsom::builder::name("W")   / cxsom::builder::name("Wc-1"), wtype, CACHE, trace, OPENED);
  auto Hc0 = cxsom::builder::variable("saved", cxsom::builder::name("H")   / cxsom::builder::name("Wc-0"), wtype, CACHE, trace, OPENED);
  auto Hc1 = cxsom::builder::variable("saved", cxsom::builder::name("H")   / cxsom::builder::name("Wc-1"), wtype, CACHE, trace, OPENED);
  auto Rc0 = cxsom::builder::variable("saved", cxsom::builder::name("RGB") / cxsom::builder::name("Wc-0"), wtype, CACHE, trace, OPENED);
  auto Rc1 = cxsom::builder::variable("saved", cxsom::builder::name("RGB") / cxsom::builder::name("Wc-1"), wtype, CACHE, trace, OPENED);
  Wmap->contextual(Hmap, fx::match_gaussian, p.match_pos, Wc0, saved_weight_at);
  Wmap->contextual(Rmap, fx::match_gaussian, p.match_pos, Wc1, saved_weight_at);
  Hmap->contextual(Wmap, fx::match_gaussian, p.match_pos, Hc0, saved_weight_at);
  Hmap->contextual(Rmap, fx::match_gaussian, p.match_pos, Hc1, saved_weight_at);
  Rmap->contextual(Wmap, fx::match_gaussian, p.match_pos, Rc0, saved_weight_at);
  Rmap->contextual(Hmap, fx::match_gaussian, p.match_pos, Rc1, saved_weight_at);

  // Let us declare the inputs (W, H) and the output (RGB).
  auto W   = cxsom::builder::variable("img"        , cxsom::builder::name("w")  , "Pos1D"  , CACHE, trace, OPENED);
  auto H   = cxsom::builder::variable("img"        , cxsom::builder::name("h")  , "Pos1D"  , CACHE, trace, OPENED);
  auto RGB = cxsom::builder::variable("predict-out", cxsom::builder::name("rgb"), "Array=3", CACHE, trace, OPENED);
  W->definition();
  H->definition();
  RGB->definition();

  // Let us provide inputs to W, and H maps, using weights learnt previously as well.
  auto We0 = cxsom::builder::variable("saved", cxsom::builder::name("W")   / cxsom::builder::name("We-0"), wtype, CACHE, trace, OPENED);
  auto He0 = cxsom::builder::variable("saved", cxsom::builder::name("H")   / cxsom::builder::name("We-0"), wtype, CACHE, trace, OPENED);  
  auto Re0 = cxsom::builder::variable("saved", cxsom::builder::name("RGB") / cxsom::builder::name("We-0"), rtype, CACHE, trace, OPENED);
  Wmap->external(W, fx::match_gaussian, p.match_ctx, We0, saved_weight_at);
  Hmap->external(H, fx::match_gaussian, p.match_ctx, He0, saved_weight_at);

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
    std::ofstream dot_file("predict.dot");
    dot_file << archi->write_dot;
  }

  // Now, we need supplementary rules for reading the RGB
  // result. Indeed, it consists in using the BMU of the RGB map to
  // index the learnt RGB weights.
  RGB->var() << fx::value_at(kwd::at(Re0->var(), saved_weight_at), Rmap->output_BMU()->var()) | kwd::use("walltime", FOREVER);
}



// ########
// #      #
// # Main #
// #      #
// ########


int main(int argc, char* argv[]) {
  context c(argc, argv);
  Mode mode;
  unsigned int walltime        = 0;
  unsigned int grid_side       = 0;
  unsigned int img_side        = 0;
  unsigned int save_period     = 0;
  unsigned int saved_weight_at = 0;
  
  // We analyse the arguments and identify the mode.
  std::ostringstream prefix;
  for(const auto& arg : c.argv) prefix << arg << ' ';
  prefix << "-- ";
  
  if(c.user_argv.size() == 0) {
    std::cout << "You have to provide user arguments." << std::endl
	      << "e.g:" << std::endl
	      << "  " << prefix.str() << "calibration <grid-side>            <-- sends the rules for calibration." << std::endl
	      << "  " << prefix.str() << "walltime <max-time>                <-- sends the rules for the inputs wall-time redefinition." << std::endl
	      << "  " << prefix.str() << "input <img-side>                   <-- sends the rules for the inputs." << std::endl
	      << "  " << prefix.str() << "train <save-period> <img-side>     <-- sends the rules for training." << std::endl
	      << "  " << prefix.str() << "check <saved-weight-at> <img-side> <-- sends the rules for checking." << std::endl
	      << "  " << prefix.str() << "predict <saved-weight-at>          <-- sends the rules for predicting." << std::endl;
    c.notify_user_argv_error(); 
    return 0;
  }

  if(c.user_argv[0] == "calibration") {
    if(c.user_argv.size() != 2) {
      std::cout << "The 'calibration' mode expects a grid_side argument"  << std::endl;
      c.notify_user_argv_error(); 
      return 0;
    }
    grid_side = stoul(c.user_argv[1]);
    mode = Mode::Calibration;
  }
  else if(c.user_argv[0] == "walltime") {
    if(c.user_argv.size() != 2) {
      std::cout << "The 'walltime' mode expects a max-time argument"  << std::endl;
      c.notify_user_argv_error(); 
      return 0;
    }
    walltime = stoul(c.user_argv[1]);
    mode = Mode::Walltime;
  }
  else if(c.user_argv[0] == "input") {
    if(c.user_argv.size() != 2) {
      std::cout << "The 'input' mode expects a img-side argument"  << std::endl;
      c.notify_user_argv_error(); 
      return 0;
    }
    img_side = stoul(c.user_argv[1]);
    mode = Mode::Input;
  }
  else if(c.user_argv[0] == "train") {
    if(c.user_argv.size() != 3) {
      std::cout << "The 'train' mode expects save-period and img-side arguments"  << std::endl;
      c.notify_user_argv_error(); 
      return 0;
    }
    save_period = stoul(c.user_argv[1]);
    img_side = stoul(c.user_argv[2]);
    mode = Mode::Train;
  }
  else if(c.user_argv[0] == "check") {
    if(c.user_argv.size() != 3) {
      std::cout << "The 'check' mode expects saved-weight-at and img-side arguments"  << std::endl;
      c.notify_user_argv_error(); 
      return 0;
    }
    saved_weight_at = stoul(c.user_argv[1]);
    img_side = stoul(c.user_argv[2]);
    mode = Mode::Check;
  }
  else if(c.user_argv[0] == "predict") {
    if(c.user_argv.size() != 3) {
      std::cout << "The 'predict' mode expects saved-weight-at and img-side arguments"  << std::endl;
      c.notify_user_argv_error(); 
      return 0;
    }
    saved_weight_at = stoul(c.user_argv[1]);
    img_side = stoul(c.user_argv[2]);
    mode = Mode::Predict;
  }
  else {
    std::cout << "Bad user arguments." << std::endl;
    c.notify_user_argv_error(); 
    return 0;
  }

  switch(mode) {
  case Mode::Calibration:
    make_calibration_rules(grid_side);
    break;
  case Mode::Walltime:
    make_walltime_rules(walltime);
    break;
  case Mode::Input:
    make_input_rules(img_side);
    break;
  case Mode::Train:
    make_train_rules(save_period, img_side);
    break;
  case Mode::Predict:
    make_predict_rules(saved_weight_at, img_side);
    break;
  case Mode::Check:
    make_check_rules(saved_weight_at, img_side);
    break;
  default:
    break;
  }
  
  return 0;
}
