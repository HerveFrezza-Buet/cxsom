#include <cxsom-viewer.hpp>

#include <iostream>
#include <iostream>

// This example pings a cxseom processor.


int main(int argc, char* argv[]) {
  if(argc < 3) {
    std::cout << "Usage : " << argv[0] << " <hostname> <port>" << std::endl
	      << std::endl;
    return 0;
  }

  std::string hostname {argv[1]};
  unsigned int port {std::stoul(argv[2])};
  try {
    cxsom::protocol::ping(hostname, port);
  }
  catch(cxsom::protocol::remote_error& e) {
    std::cout << "Server is not ok with the ping: " << e.what() << std::endl;
  }
  catch(std::exception& e) {
    std::cout << "Something went wrong: " << e.what() << std::endl;
  }

  return  0;
}
