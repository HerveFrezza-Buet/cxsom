#pragma once

#include <iostream>

#include <skednetProtocol.hpp>

namespace sked {
  namespace net {
    namespace scope {
      namespace xrsw {
	
	struct read : public sked::net::protocol::scope::client_plug {
	  read(std::istream& is, std::ostream& os): sked::net::protocol::scope::client_plug('R', 'r', is, os) {} 
	};
	
	struct write : public sked::net::protocol::scope::client_plug {
	  write(std::istream& is, std::ostream& os): sked::net::protocol::scope::client_plug('W', 'w', is, os) {} 
	};
	
	struct write_explicit : public sked::net::protocol::scope::client_plug_explicit {
	  write_explicit(std::istream& is, std::ostream& os): sked::net::protocol::scope::client_plug_explicit('W', 'w', is, os) {}
	};
	
      }
    }
  }
}
