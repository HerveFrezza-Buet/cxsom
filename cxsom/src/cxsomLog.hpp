#pragma once
#include <iostream>
#include <string>
#include <sstream>

#include <cxsomSymbols.hpp>

namespace cxsom {
  class Log;

  extern Log* logger;
  
  class Log {

  private:

    std::ostream* os;

    std::string indent() {return std::string(indentation*2, ' ');}

  public:

    std::size_t indentation = 0;
    
    Log() : os(&std::cout), indentation(0)  {}

    void push() {++indentation;}
    void pop() {--indentation;}
    void _msg(const std::string& m) {
      push();
      msg(m);
      pop();
    }
    
    void msg(const std::string& m) {
      std::istringstream istr(m);
      while(!istr.eof()) {
	std::string line;
	std::getline(istr, line, '\n');
	*os << indent() << line << std::endl;
      }
    }
  };

  
  class Tick;

  extern Tick* ticker;
  
  class Tick {

  private:

    std::size_t time = 0;

  public:

    Tick() : time(0) {}

    std::size_t t() {return time++;}
  };
}
