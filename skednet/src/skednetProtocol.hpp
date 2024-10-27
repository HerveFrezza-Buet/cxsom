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
	/**
	 * The protocol consists in two server-client
	 * interactions. First the client sends a request (a single
	 * char). It waits the answer from the server (another
	 * char). The server reads the request, and unlock the waiting
	 * client by sending the char it expects. Then the client gets
	 * unlocked and it makes some jobs. When that job is finished,
	 * the client triggers another interaction similarily, in
	 * other to notify the server.
	 *
	 * Plugs handle this exchange at each side.
	 */
	struct plug {
	  char server_tag, client_tag;
	  std::istream& is;
	  std::ostream& os;
	  plug(char server_tag, char client_tag, std::istream& is, std::ostream& os): server_tag(server_tag), client_tag(client_tag), is(is), os(os) {}
	};

	struct server_plug : public plug {
	  using plug::plug;
	  /**
	   * This acknowledges the client.
	   */
	  void operator()() {
	    char c;
	    os << server_tag << std::endl;
	    is >> c;
	    if(c != client_tag) throw sked::net::exception::protocol_error(std::string("server_plug: bad tag received."));
	    os << server_tag << std::endl;
	  }
	};
	
	/**
	 * The client plug has two methods, enter and leave, for each
	 * interaction. This client is explicit since, once created,
	 * the user needs to call enter and leave explicitly.
	 */
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

	/**
	 * This client plug performs the first interaction in the
	 * constructor, and the second in the destructor.
	 */
	struct client_plug : private client_plug_explicit {
	  client_plug(char server_tag, char client_tag, std::istream& is, std::ostream& os): client_plug_explicit(server_tag, client_tag, is, os) {enter();}
	  ~client_plug() noexcept(false) {leave();}
	};
	  
      }
    }
  }
}
