
// This executable is a cxsom processor, where custom "foo" stuff is
// made available.


// Uncomment this for debugging verbosity.
// #define cxsomDEBUG_PROTOCOL
// #define cxsomLOG
#include <cxsom-processor.hpp>                // For basic cxsom stuff.
#include "example-005-001-foo-operations.hpp" // for foo stuff.


// This is used in case of some of the previous macros are set. You
// can remove this lines otherwise... or keep it.
cxsom::Tick* cxsom::ticker = new cxsom::Tick();
cxsom::Log*  cxsom::logger = new cxsom::Log();


int main(int argc, char* argv[]) try {
  if(argc != 4) {
    std::cout << "Usage : " << argv[0] << " <root-dir> <nb_threads> <port>" << std::endl;
    return 0;
  }

  std::string root_dir =           argv[1] ;
  int nb_threads       = std::stoi(argv[2]);
  int port             = std::stoi(argv[3]);

  cxsom::jobs::UpdateFactory update_factory;  // This is the factory producing updates.
  cxsom::jobs::fill(update_factory);          // We add the basic cxsom updates.
  foo::fill(update_factory);                  // We add the foo updates.
  
  cxsom::jobs::TypeChecker type_checker;      // This is the type checker.
  cxsom::jobs::fill(type_checker);            // We add the cxsom updates type checkings.
  foo::fill(type_checker);                    // We add the foo updates type checkings.

  // Now we launch the processor.
  cxsom::processor::launch(update_factory, type_checker, root_dir, nb_threads, port);
  
  return 0;
 }
 catch (std::exception& e) {
   std::cerr << "Exception : " << e.what() << std::endl;
   return 1;
 }
