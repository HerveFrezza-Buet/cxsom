#include <cxsom-rules.hpp>

using namespace cxsom::rules;
context* cxsom::rules::ctx = nullptr;


#define WALLTIME    10000 
#define CACHE_SIZE      2 
#define BUF_SIZE    (WALLTIME+1) 
#define KEPT_OPENED  true


int main(int argc, char* argv[]) {
  context c(argc, argv);
  
  if(c.user_argv.size() != 0) {
    std::cout << "You are not allowed to provide user arguments." << std::endl;
    c.notify_user_argv_error(); // Do this to avoid irrelevant default behavior when your program exists.
    return 0;
  }

  {
    timeline t("in"); // We work in the timeline "main".
    
    kwd::type("I", "Map1D<Scalar>=500", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    "I" << fx::random() | kwd::use("walltime", WALLTIME);
  }
  
  {
    timeline t("argmax"); // We work in the timeline "main".
    
    kwd::type("bmu", "Pos1D", CACHE_SIZE, BUF_SIZE, KEPT_OPENED);
    "bmu" << fx::argmax(kwd::var("in", "I"))  | kwd::use("walltime", WALLTIME), kwd::use("random-bmu", 1);
  }
  
  return 0;
}
