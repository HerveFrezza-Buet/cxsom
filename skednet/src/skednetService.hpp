#pragma once

#include <thread>
#include <chrono>
#include <sked.hpp>

#include <skednetProtocol.hpp>

namespace sked {
  namespace net {
    namespace xrsw {
      class main {
      public:
	sked::xrsw::queue queue;
	template <typename Duration>
	void loop(const Duration& check_period) {
	  while(true) {
	    while(!queue.flush()) std::this_thread::sleep_for(check_period);
	    while(queue.flush());
	  }
	}
      };
    }
    
    class core_service {
      std::shared_ptr<asio::ip::tcp::iostream>  p_socket;
    public:
      core_service(asio::ip::tcp::acceptor& acceptor) 
	: p_socket(new asio::ip::tcp::iostream()) {
	acceptor.accept(*(p_socket->rdbuf())); // Blocking is here
      }
      
    protected:
      asio::ip::tcp::iostream& socket() {return *p_socket;}
    };

    namespace xrsw {
    
      class service : public core_service {
      private:

	main& context;
	
      public:

	service(main& context, asio::ip::tcp::acceptor& acceptor) : core_service(acceptor), context(context) {}
	  
	void operator()() {
	  try {
	    socket().exceptions(std::ios::failbit | std::ios::badbit | std::ios::eofbit);
	    sked::net::protocol::scope::server_plug read_plug('R', 'r',  socket(), socket());
	    sked::net::protocol::scope::server_plug write_plug('W', 'w', socket(), socket());
	    while(true) {
	      char c;
	      socket() >> c;
	      if(c == 'r') {
		auto lock = sked::xrsw::reader_scope(context.queue);
		read_plug();
	      }
	      else if (c == 'w') {
		auto lock = sked::xrsw::writer_scope(context.queue);
		write_plug();
	      }
	      else
		throw sked::net::exception::protocol_error("sked::net::xrsw::service: bad tag.");
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
	
    }
  }
}
