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

#define CACHE              2
#define UNDEFINED_TRACE 1000
#define FROZEN_TRACE    1000
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

enum class Mode : char {Train, Test, Input};


void make_input_rules(unsigned int walltime) {
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
	      << "  " << prefix.str() << "input <max-time> <-- sends the rules for the inputs." << std::endl
	      << "  " << prefix.str() << "train            <-- sends the rules for training." << std::endl
	      << "  " << prefix.str() << "test             <-- sends the rules for testing." << std::endl;
    c.notify_user_argv_error(); 
    return 0;
  }

  if(c.user_argv[0] == "input") {
    if(c.user_argv.size() != 2) {
      std::cout << "The 'input' mode expects a max-time argument"  << std::endl;
      c.notify_user_argv_error(); 
      return 0;
    }
    walltime = stoul(c.user_argv[1]);
    mode = Mode::Input;
  }
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
    make_input_rules(walltime);
  case Mode::Train:
  case Mode::Test:
  default:
    break;
  }
  
  return 0;
}
