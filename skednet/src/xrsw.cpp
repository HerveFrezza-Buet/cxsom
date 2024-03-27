
#include <iostream>
#include <thread>
#include <memory>
#include <chrono>

#include <utility> // should be included by asio
#include <asio.hpp>

#include <skednet.hpp>


using namespace std::literals::chrono_literals;

int main(int argc, char* argv[]) {

  if(argc != 2) {
    std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
    return 0;
  }
  
  asio::io_service        ios;
  sked::net::xrsw::main context;

  std::thread listen {[&ios, &context, port = std::stoi(argv[1])](){
    asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);
    asio::ip::tcp::acceptor acceptor(ios, endpoint);
    while(true) {
      std::thread service {sked::net::xrsw::service(context, acceptor)};
      service.detach();
    }
  }};

  context.loop(500ms);

  listen.join();
  return 0;
}
