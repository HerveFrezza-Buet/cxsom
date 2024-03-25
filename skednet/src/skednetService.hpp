#pragma once

#include <sked.hpp>

namespace sked {
  namespace net {

    namespace main {
      class xrsw {
      public:
	void loop() {}
      };
    }
    
    namespace service {

      class core {
	std::shared_ptr<asio::ip::tcp::iostream>  p_socket;
      public:
	core(asio::ip::tcp::acceptor& acceptor) 
	  : p_socket(new asio::ip::tcp::iostream()) {
	  acceptor.accept(*(p_socket->rdbuf())); // Blocking is here
	}

      protected:
	asio::ip::tcp::iostream& socket() {return *p_socket;}
      };

      class xrsw : public core {
      private:

	main::xrsw& context;
	
      public:

	xrsw(main::xrsw& context, asio::ip::tcp::acceptor& acceptor) : core(acceptor), context(context) {}
	  
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

	void loop() {
	}
      };
	
    }
  }
}
