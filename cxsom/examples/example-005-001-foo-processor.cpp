
// This executable is a cxsom processor, where custom "foo" stuff is
// made available.


// Uncomment this for debugging verbosity.
// #define cxsomDEBUG_PROTOCOL
// #define cxsomLOG
// #define cxsomMONITOR
#include <cxsom-processor.hpp>                // For basic cxsom stuff.
#include "example-005-001-foo-operations.hpp" // for foo stuff.



// This is used in case of some macros are set. You can remove this
// lines otherwise... or keep it.

cxsom::Tick* cxsom::ticker  = new cxsom::Tick();

#ifdef cxsomLOG
cxsom::Log* cxsom::logger = new cxsom::Log();
#else
cxsom::Log* cxsom::logger = nullptr;
#endif

#ifdef cxsomMONITOR
cxsom::Monitor*  cxsom::monitor = new cxsom::Monitor();
#else
cxsom::Monitor*  cxsom::monitor = nullptr;
#endif


int main(int argc, char* argv[]) try {
  if(argc != 4) {
    std::cout << "Usage : " << argv[0] << " <root-dir> <nb_threads> <port>" << std::endl;
    return 0;
  }

  std::string root_dir =           argv[1] ;
  int nb_threads       = std::stoi(argv[2]);
  int port             = std::stoi(argv[3]);

#ifdef cxsomDEBUG_PROTOCOL
  nb_threads = 1;
#endif
#ifdef cxsomLOG
  nb_threads = 1;
#endif

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
