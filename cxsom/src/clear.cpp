#include <string>
#include <iostream>
#include <boost/asio.hpp>

int main(int argc, char* argv[]) {
  if(argc != 3) {
    std::cout << "Usage : " << argv[0] << " <hostname> <port>" << std::endl;
    return 0;
  }

  std::string hostname(argv[1]);
  std::string port    (argv[2]);
  
  boost::asio::ip::tcp::iostream socket;
  socket.exceptions(std::ios::failbit | std::ios::badbit | std::ios::eofbit);

  try {
    socket.connect(hostname, port);

    socket << "clear\n" << std::flush;
  
    std::string line;
    std::getline(socket, line, '\n');
    if(line != "ok")
      std::cerr << line << std::endl;
  }
  catch(std::exception& e) {
    std::cerr << "Exception caught : " << e.what() << " --> " << typeid(e).name()<< std::endl;
  }
  
  
  return 0;
}
