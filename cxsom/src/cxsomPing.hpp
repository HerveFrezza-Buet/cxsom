#pragma once

#include <string>
#include <utility> // should be included by asio
#include <stdexcept>
#include <asio.hpp>

namespace cxsom {
  namespace protocol {
    struct remote_error : public std::runtime_error {
      using std::runtime_error::runtime_error;
    };
    
    inline void ping(const std::string& hostname, unsigned int port) {
      asio::ip::tcp::iostream socket;
      socket.exceptions(std::ios::failbit | std::ios::badbit | std::ios::eofbit);
      socket.connect(hostname, std::to_string(port));
      socket << "ping\n" << std::flush;
      std::string line;
      std::getline(socket, line, '\n');
      if(line != "ok")
	throw remote_error("Bad ping response");
    }
  }
}
