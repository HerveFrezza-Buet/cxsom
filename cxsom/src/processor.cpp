
// #define cxsomDEBUG_PROTOCOL
// #define cxsomLOG
#include <cxsom-server.hpp>

// This is used in case of some macros are set. You can remove this
// lines otherwise... or keep it.
cxsom::Tick* cxsom::ticker = new cxsom::Tick();
cxsom::Log*  cxsom::logger = new cxsom::Log();


#include <random>
#include <thread>
#include <string>
#include <memory>
#include <stdexcept>
#include <boost/asio.hpp>
#include <sstream>
#include <iomanip>

class ServiceThread {
private:

  cxsom::data::Center& data_center;
  cxsom::jobs::Center& jobs_center;
  std::shared_ptr<boost::asio::ip::tcp::iostream>  p_socket;

  void process_ping() {
#ifdef cxsomDEBUG_PROTOCOL
    std::cout << ">> process_ping() >>>>" << std::endl;
#endif
    jobs_center.interaction_lock();
    jobs_center.clear_blocking_info();
    *p_socket << "ok" << std::endl;
    jobs_center.interaction_release();
#ifdef cxsomDEBUG_PROTOCOL
    std::cout << "-- process_ping <<<<" << std::endl;
#endif
  }
  
  void process_clear() {
#ifdef cxsomDEBUG_PROTOCOL
    std::cout << ">> process_clear() >>>>" << std::endl;
#endif
    jobs_center.interaction_lock();
    data_center.clear();
    jobs_center.clear();
    *p_socket << "ok" << std::endl;
    jobs_center.interaction_release();
#ifdef cxsomDEBUG_PROTOCOL
    std::cout << "-- process_clear <<<<" << std::endl;
#endif
  }
  
  void process_declare() {
#ifdef cxsomDEBUG_PROTOCOL
    std::string buf;
    std::getline(*p_socket, buf);
    std::istringstream socket(buf);
    std::cout << ">> " << buf << " // process_declare(...) >>>>" << std::endl;
#else
    boost::asio::ip::tcp::iostream& socket = *p_socket;
#endif
    try {
      auto v = cxsom::protocol::read::variable(socket);
#ifdef cxsomDEBUG_PROTOCOL
      std::cout << "-- got variable " << v << std::endl;
#endif
      auto t = cxsom::protocol::read::type_value(socket);
#ifdef cxsomDEBUG_PROTOCOL
      std::cout << "-- got type " << t;
      if(t)
	std::cout << " = " << t->name() << std::endl;
      else
	std::cout << " = ???" << std::endl;
#endif
      auto [cache_size, file_size, kept_opened] = cxsom::protocol::read::storage(socket);
#ifdef cxsomDEBUG_PROTOCOL
      std::cout << "-- got cache_size  = " << cache_size << std::endl
		<< "       file_size   = " << file_size << std::endl
		<< "       kept_opened = " << std::boolalpha << kept_opened << std::endl;
#endif
      if(cache_size > 0)
	data_center.check(v, t, cache_size, file_size, kept_opened);
      else
	data_center.check(v, t, kept_opened);
      *p_socket << "ok" << std::endl;
#ifdef cxsomDEBUG_PROTOCOL
      std::cout << "-- process_declare <<<<" << std::endl;
#endif
    }
    catch(const std::exception& e) {
#ifdef cxsomDEBUG_PROTOCOL
      std::cout << "!! " << e.what() << std::endl;
#endif
      *p_socket << "error " << e.what() << std::endl;
    }
  }

  std::size_t nb_updates() {
    std::size_t res;
#ifdef cxsomDEBUG_PROTOCOL
    std::string buf;
    std::getline(*p_socket, buf);
    std::istringstream socket(buf);
    std::cout << ">> " << buf << " // the number of updates" << std::endl;
    socket >> res;
#else
    char c;
    auto& socket = *p_socket;
    socket >> res;
    socket.get(c); // ' '
    socket.get(c); // '\n'
#endif
    return res;
  }
  
  void process_updates() {
    boost::asio::ip::tcp::iostream& socket = *p_socket;
    std::string command;
    char c;
    std::size_t number = nb_updates();
    jobs_center.interaction_lock();
    for(std::size_t i = 0; i < number; ++i) {
      socket >> command;
      socket.get(c);
      if     (command == "static" ) process_static();
      else if(command == "pattern") process_pattern();
      else
	socket << "error expecting \"static\" or \"pattern\", got \"" << command << "\" instead." << std::endl;
    }
    jobs_center.interaction_release();
  }
	    
  void process_static() {
#ifdef cxsomDEBUG_PROTOCOL
    std::string buf;
    std::getline(*p_socket, buf);
    std::istringstream socket(buf);
    std::cout << ">> " << buf << std::endl;
#else
    boost::asio::ip::tcp::iostream& socket = *p_socket;
#endif
    try {
      jobs_center += cxsom::protocol::read::update(socket);
      *p_socket << "ok" << std::endl;
    }
    catch(const std::exception& e) {
#ifdef cxsomDEBUG_PROTOCOL
      std::cout << "!! " << e.what() << std::endl;
#endif
      *p_socket << "error " << e.what() << std::endl;
    }
  }
  
  void process_pattern() {
#ifdef cxsomDEBUG_PROTOCOL
    std::string buf;
    std::getline(*p_socket, buf);
    std::istringstream socket(buf);
    std::cout << ">> " << buf << std::endl;
#else
    boost::asio::ip::tcp::iostream& socket = *p_socket;
#endif
    try {
      jobs_center += cxsom::protocol::read::pattern::update(socket);
      *p_socket << "ok" << std::endl;
    }
    catch(const std::exception& e) {
#ifdef cxsomDEBUG_PROTOCOL
      std::cout << "!! " << e.what() << std::endl;
#endif
      *p_socket << "error " << e.what() << std::endl;
    }
  }

public:

  ServiceThread(cxsom::data::Center& data_center,
		cxsom::jobs::Center& jobs_center,
		boost::asio::ip::tcp::acceptor& acceptor) 
    : data_center(data_center), jobs_center(jobs_center), p_socket(new boost::asio::ip::tcp::iostream()) {
    acceptor.accept(*(p_socket->rdbuf())); // Blocking is here
  }
  ServiceThread(const ServiceThread& cp) = default;

  void operator()(void) {
    boost::asio::ip::tcp::iostream& socket = *p_socket;
    std::string command;
    char c;
    try {
      socket.exceptions(std::ios::failbit | std::ios::badbit | std::ios::eofbit);
      while(true) {
	socket >> command;
	socket.get(c);
	if     (command == "declare") process_declare();
	else if(command == "updates") process_updates();
	else if(command == "ping"   ) process_ping();
	else if(command == "clear"  ) process_clear();
	else
	  socket << "error command \"" << command << "\" not implemented." << std::endl;
      }
    }
    catch(std::exception& e) {
      std::cout << "Exception caught : " << e.what() << std::endl;
    }
  }
};


int main(int argc, char* argv[]) try {
  if(argc != 4) {
    std::cout << "Usage : " << argv[0] << " <root-dir> <nb_threads> <port>" << std::endl;
    return 0;
  }

  int nb_threads = std::stoi(argv[2]);
  
  std::random_device rd;
  cxsom::data::Center data_center(argv[1]);
  cxsom::jobs::Center jobs_center(rd, data_center);
  
  std::vector<std::thread> workers;
  for(int i = 0; i < nb_threads; ++i)
    workers.emplace_back([&jobs_center](){jobs_center.worker_thread();});
  
  boost::asio::io_service        ios;
  boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), std::stoi(argv[3]));
  boost::asio::ip::tcp::acceptor acceptor(ios, endpoint);

  while(true) {
    std::thread service(ServiceThread(data_center, jobs_center, acceptor));
    service.detach();
  }
  
  for(auto& w : workers) w.join();
  return 0;
}
 catch (std::exception& e) {
   std::cerr << "Exception : " << e.what() << std::endl;
   return 1;
 }
