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

#define CACHE              2
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

void make_input_rules() {
  auto SRC = cxsom::builder::variable("img", cxsom::builder::name("src"),   img_type(),     1,     1, OPENED);
  auto PXL = cxsom::builder::variable("img", cxsom::builder::name("coord"),    "Pos2D", CACHE,     1, OPENED);
  auto [W, H, RGB] = rgb_inputs();
  SRC->definition();
  PXL->definition();
  W->definition();
  H->definition();
  RGB->definition();

  auto src = SRC->varname.value;
  auto pxl = PXL->varname.value;
  auto w   = W->varname.value;
  auto h   = H->varname.value;
  auto rgb = RGB->varname.value;

  {
    timeline t{"img"};
    pxl << fx::random() | kwd::use("walltime", 0);
  }
  
  {
    timeline t{"in"};
    w   << fx::first (kwd::var("img", pxl))                                     | kwd::use("walltime", FOREVER);
    h   << fx::second(kwd::var("img", pxl))                                     | kwd::use("walltime", FOREVER);
    rgb << fx::value_at(kwd::at(kwd::var("img", src), 0), kwd::var("img", pxl)) | kwd::use("walltime", FOREVER);
  }
    
}

int main(int argc, char* argv[]) {
  context c(argc, argv);
  Mode mode;
  unsigned int walltime = 0;
  
  // We analyse the arguments and identify the mode.
  std::ostringstream prefix;
  for(const auto& arg : c.argv) prefix << arg << ' ';
  prefix << "-- ";
  
  if(c.user_argv.size() == 0) {
    std::cout << "You have to provide user arguments." << std::endl
	      << "e.g:" << std::endl
	      << "  " << prefix.str() << "input               <-- sends the rules for the inputs." << std::endl
	      << "  " << prefix.str() << "walltime <max-time> <-- sends the rules for the inputs wall-time redefinition." << std::endl
	      << "  " << prefix.str() << "train               <-- sends the rules for training." << std::endl
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
  else if(c.user_argv[0] == "train")
    mode = Mode::Train;
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
  case Mode::Walltime:
  case Mode::Train:
  case Mode::Test:
  default:
    break;
  }
  
  return 0;
}
