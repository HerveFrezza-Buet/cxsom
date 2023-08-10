#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

#include <mutex>

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


  class Monitor;

  extern Monitor* monitor;

  /*

    INFO_LINE := JOB_LINE | TIMESTEP_LINE
    TIMESTEP_LINE := timestep <timeline> <at> TIMESTEP_INFO
    JOB_LINE := job JOB_INFO

    JOB_INFO := remove TIMESTEP

   */
  class Monitor {
  private:

    mutable std::mutex mutex;
    mutable std::ofstream out;

    void sep() const {
      out << ' ';
    }
    
    void eol() const {
      out << std::endl;
    }
    
    void tag(const std::string& t) const {
      out << t;
    }
    

    void job_header() const {
      out << "job";
    }

    void timestep(const symbol::TimeStep& ts) const {
      out << ts.timeline << ' ' << ts.at;
    }
    
  public:

    Monitor() : out("monitoring.data") {}

    void job_remove_timestep(const symbol::TimeStep& ts) const {
      std::lock_guard<std::mutex> lock(mutex);
      job_header(); sep(); tag("remove"); sep(); timestep(ts); eol();
    }
  };
}
