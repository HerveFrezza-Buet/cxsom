#pragma once

#include <thread>
#include <chrono>
#include <sked.hpp>

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
      core(asio::ip::tcp::acceptor& acceptor) 
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

	xrsw(main& context, asio::ip::tcp::acceptor& acceptor) : core_service(acceptor), context(context) {}
	  
	void operator()() {
	  try {
	    socket().exceptions(std::ios::failbit | std::ios::badbit | std::ios::eofbit);
	    while(true) {
	      // socket() >> command;
	      // socket().get(c);
	      // socket() << command.size() << std::endl;
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
