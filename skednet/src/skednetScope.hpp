#pragma once

#include <iostream>

#include <skednetProtocol.hpp>

namespace sked {
  namespace net {
    namespace scope {
      namespace xrsw {

	/**
	 * @short Client plug sending 'R' and waiting for 'r' as an anwer.
	 */
	struct read : public sked::net::protocol::scope::client_plug {
	  read(std::istream& is, std::ostream& os): sked::net::protocol::scope::client_plug('R', 'r', is, os) {} 
	};
	
	/**
	 * @short Client plug sending 'W' and waiting for 'w' as an anwer.
	 */
	struct write : public sked::net::protocol::scope::client_plug {
	  write(std::istream& is, std::ostream& os): sked::net::protocol::scope::client_plug('W', 'w', is, os) {} 
	};
	
	/**
	 * @short Explicit client plug sending 'W' and waiting for 'w' as an anwer.
	 */
	struct write_explicit : public sked::net::protocol::scope::client_plug_explicit {
	  write_explicit(std::istream& is, std::ostream& os): sked::net::protocol::scope::client_plug_explicit('W', 'w', is, os) {}
	};
	
      }
    }
  }
}
