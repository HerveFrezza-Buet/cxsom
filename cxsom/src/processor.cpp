
#include <utility> // should be included by asio
#include <asio.hpp>
#include <skednet.hpp>
#include <memory>


// #define cxsomDEBUG_PROTOCOL
// #define cxsomLOG
// #define cxsomMONITOR
#include <cxsom-processor.hpp>

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
  if(argc != 4 && argc != 6) {
    std::cout << "Usage : " << argv[0] << " <root-dir> <nb_threads> <port> [<xrsw-hostname> <xrsw-port>]" << std::endl;
    return 0;
  }

  std::string root_dir =           argv[1] ;
  int nb_threads       = std::stoi(argv[2]);
  int port             = std::stoi(argv[3]);
  
  std::shared_ptr<sked::net::scope::xrsw::write_explicit> xrsw_writer;
  asio::ip::tcp::iostream socket;
  if(argc == 6) {
    socket.exceptions(std::ios::failbit | std::ios::badbit | std::ios::eofbit);
    socket.connect(argv[4], argv[5]);
    xrsw_writer = std::make_shared<sked::net::scope::xrsw::write_explicit>(socket, socket);
  }

#ifdef cxsomDEBUG_PROTOCOL
  nb_threads = 1;
#endif
#ifdef cxsomLOG
  nb_threads = 1;
#endif
#ifdef cxsomMONITOR
  nb_threads = 1;
#endif

  cxsom::jobs::UpdateFactory update_factory;
  cxsom::jobs::fill(update_factory);
  
  cxsom::jobs::TypeChecker type_checker;
  cxsom::jobs::fill(type_checker);

  cxsom::processor::launch(update_factory, type_checker, root_dir, nb_threads, port, xrsw_writer);
  
  return 0;
 }
 catch (std::exception& e) {
   std::cerr << "Exception : " << e.what() << std::endl;
   return 1;
 }
