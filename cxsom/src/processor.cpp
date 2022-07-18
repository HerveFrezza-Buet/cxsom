
// #define cxsomDEBUG_PROTOCOL
// #define cxsomLOG
#include <cxsom-processor.hpp>

// This is used in case of some macros are set. You can remove this
// lines otherwise... or keep it.
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

  cxsom::jobs::UpdateFactory update_factory;
  cxsom::jobs::fill(update_factory);
  
  cxsom::jobs::TypeChecker type_checker;
  cxsom::jobs::fill(type_checker);

  cxsom::processor::launch(update_factory, type_checker, root_dir, nb_threads, port);
  
  return 0;
 }
 catch (std::exception& e) {
   std::cerr << "Exception : " << e.what() << std::endl;
   return 1;
 }
