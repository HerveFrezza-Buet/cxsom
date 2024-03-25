

#include <iostream>
#include <thread>
#include <memory>

#include <utility> // should be included by asio
#include <asio.hpp>

class ServiceThread {
  std::shared_ptr<asio::ip::tcp::iostream>  p_socket;

public:
  ServiceThread(asio::ip::tcp::acceptor& acceptor) 
    : p_socket(new asio::ip::tcp::iostream()) {
    acceptor.accept(*(p_socket->rdbuf())); // Blocking is here
  }
  ServiceThread(const ServiceThread& cp) = default;
  
  void operator()(void) {
    asio::ip::tcp::iostream& socket = *p_socket;
    std::string command;
    char c;
    try {
      socket.exceptions(std::ios::failbit | std::ios::badbit | std::ios::eofbit);
      while(true) {
	socket >> command;
	socket.get(c);
	socket << command.size() << std::endl;
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

int main(int argc, char* argv[]) {
  asio::io_service        ios;
  asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), 10000);
  asio::ip::tcp::acceptor acceptor(ios, endpoint);
  
  while(true) {
    std::thread service {ServiceThread(acceptor)};
    service.detach();
  }

  return 0;
}
