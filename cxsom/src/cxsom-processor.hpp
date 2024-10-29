#pragma once


#include <random>
#include <thread>
#include <string>
#include <memory>
#include <stdexcept>

#include <utility> // should be included by asio
#include <asio.hpp>

#include <sstream>
#include <iomanip>

#include <skednet.hpp>
#include <cxsom-server.hpp>

namespace cxsom {
  namespace processor {

    class ServiceThread {
    private:

      cxsom::data::Center& data_center;
      cxsom::jobs::Center& jobs_center;
      std::shared_ptr<asio::ip::tcp::iostream>  p_socket;

      void process_ping() {
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << ">> process_ping() >>>>" << std::endl;
#endif
	jobs_center.interaction_lock();
	jobs_center.clear_blocking_info();
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << "-> answering OK." << std::endl;
#endif
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
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << "-> answering OK." << std::endl;
#endif
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
	asio::ip::tcp::iostream& socket = *p_socket;
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
#ifdef cxsomDEBUG_PROTOCOL
	  std::cout << "-> answering OK." << std::endl;
#endif
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
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << ">>  // process_updates() >>>>" << std::endl;
#endif
	asio::ip::tcp::iostream& socket = *p_socket;
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
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << "-- process_updates" << std::endl;
#endif
      }
	    
      void process_static() {
#ifdef cxsomDEBUG_PROTOCOL
	std::string buf;
	std::getline(*p_socket, buf);
	std::istringstream socket(buf);
	std::cout << ">> " << buf << "// process_static() >>>> " << std::endl;
#else
	asio::ip::tcp::iostream& socket = *p_socket;
#endif
	try {
	  jobs_center += cxsom::protocol::read::update(socket);
#ifdef cxsomDEBUG_PROTOCOL
	  std::cout << "-> answering OK." << std::endl;
#endif
	  *p_socket << "ok" << std::endl;
	}
	catch(const std::exception& e) {
#ifdef cxsomDEBUG_PROTOCOL
	  std::cout << "!! " << e.what() << std::endl;
#endif
	  *p_socket << "error " << e.what() << std::endl;
	}
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << "-- process_static" << std::endl;
#endif
      }
  
      void process_pattern() {
#ifdef cxsomDEBUG_PROTOCOL
	std::string buf;
	std::getline(*p_socket, buf);
	std::istringstream socket(buf);
	std::cout << ">> " << buf << "// process_pattern() >>>> " << std::endl;
#else
	asio::ip::tcp::iostream& socket = *p_socket;
#endif
	try {
	  jobs_center += cxsom::protocol::read::pattern::update(socket);
#ifdef cxsomDEBUG_PROTOCOL
	  std::cout << "-> answering OK." << std::endl;
#endif
	  *p_socket << "ok" << std::endl;
	}
	catch(const std::exception& e) {
#ifdef cxsomDEBUG_PROTOCOL
	  std::cout << "!! " << e.what() << std::endl;
#endif
	  *p_socket << "error " << e.what() << std::endl;
	}
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << "-- process_pattern" << std::endl;
#endif
      }

    public:

      ServiceThread(cxsom::data::Center& data_center,
		    cxsom::jobs::Center& jobs_center,
		    asio::ip::tcp::acceptor& acceptor) 
	: data_center(data_center), jobs_center(jobs_center), p_socket(new asio::ip::tcp::iostream()) {
	acceptor.accept(*(p_socket->rdbuf())); // Blocking is here
      }
      ServiceThread(const ServiceThread& cp) = default;

      void operator()() {
	asio::ip::tcp::iostream& socket = *p_socket;
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
	catch(std::ios_base::failure& e) {
      
	  // I ignore this. It occurs systematically without generating any further troubles.
	}
	catch(std::exception& e) {
	  std::cout << "Exception caught : " << e.what() << " --> " << typeid(e).name() << std::endl;
	}
      }
    };

    inline void launch(const cxsom::jobs::UpdateFactory& factory,
		       const cxsom::jobs::TypeChecker& checker,
		       const std::string& root_dir,
		       int nb_threads,
		       int port,
		       std::shared_ptr<sked::net::scope::xrsw::write_explicit> xrsw_writer) {
      std::random_device rd;
      cxsom::data::Center data_center(root_dir);
      cxsom::jobs::Center jobs_center(rd, factory, checker, data_center, xrsw_writer);
      
      std::vector<std::thread> workers;
      for(int i = 0; i < nb_threads; ++i)
	workers.emplace_back([&jobs_center](){jobs_center.worker_thread();});
      
      asio::io_service        ios;
      asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);
      asio::ip::tcp::acceptor acceptor(ios, endpoint);
      
      while(true) {
	std::thread service(ServiceThread(data_center, jobs_center, acceptor));
	service.detach();
      }
      
      for(auto& w : workers) w.join();
    }

  }
}
