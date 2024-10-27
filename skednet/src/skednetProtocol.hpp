#pragma once


#include <iostream>
#include <stdexcept>

namespace sked {
  namespace net {
    namespace exception {
      class protocol_error : public std::runtime_error {
      public:
	using std::runtime_error::runtime_error;
      };

    }
    
    namespace protocol {
      namespace scope {
	struct plug {
	  char server_tag, client_tag;
	  std::istream& is;
	  std::ostream& os;
	  plug(char server_tag, char client_tag, std::istream& is, std::ostream& os): server_tag(server_tag), client_tag(client_tag), is(is), os(os) {}
	};

	struct server_plug : public plug {
	  using plug::plug;
	  void operator()() {
	    char c;
	    os << server_tag << std::endl;
	    is >> c;
	    if(c != client_tag) throw sked::net::exception::protocol_error(std::string("server_plug: bad tag received."));
	    os << server_tag << std::endl;
	  }
	};
	

	struct client_plug_explicit : public plug {
	private:
	  void interaction() noexcept(false) try {
	    char c;
	    os << client_tag << std::endl;
	    is >> c;
	    if(c != server_tag) throw sked::net::exception::protocol_error(std::string("client_plug: bad tag received at opening."));
	  }
	  catch(sked::net::exception::protocol_error& e) {throw;}
	  catch(...) {}
	public:
	  client_plug_explicit(char server_tag, char client_tag, std::istream& is, std::ostream& os): plug(server_tag, client_tag, is, os) {}
	  void enter() {interaction();}
	  void leave() {interaction();}
	};
	
	struct client_plug : private client_plug_explicit {
	  client_plug(char server_tag, char client_tag, std::istream& is, std::ostream& os): client_plug_explicit(server_tag, client_tag, is, os) {enter();}
	  ~client_plug() noexcept(false) {leave();}
	};
	  
      }
    }
  }
}
