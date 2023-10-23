#pragma once

#include <chrono>
#include <iostream>
#include <iomanip>
#include <string>
#include <syncstream>
#include <thread>
#include <fstream>
#include <map>
#include <vector>

namespace sked {

  namespace verbose {
    struct timer {
      std::chrono::steady_clock::time_point start;
      timer() : start(std::chrono::steady_clock::now()) {}

      double operator()() const {
	std::chrono::duration<double> elapsed {std::chrono::steady_clock::now() - start};
	return elapsed.count();
      }
    };

    std::ostream& operator<<(std::ostream& os, const timer& t) {
      os.precision(3);
      return os << std::setw(7) << std::fixed << t() << std::setfill(' ');
    }

    void message(timer& t, const std::string& tag, const std::string& msg,
		 unsigned int duration_seconds) {
      std::string duration = "";
      if(duration_seconds > 0) {
	duration += " \e[32m(";
	duration += std::to_string(duration_seconds);
	duration += "s)\e[0m";
      }
      std::osyncstream(std::cout) << "\e[0;90m" << t << "\e[0m : "
				  << "\e[1;94m" << tag << "\e[0m : " 
				  << msg
				  << duration << std::endl;
      if(duration_seconds > 0)
	std::this_thread::sleep_for(std::chrono::seconds(duration_seconds));    
    }

    void message(timer& t, unsigned int thrd_id, const std::string& msg,
		 unsigned int duration_seconds) {
      std::ostringstream os;
      os << "Thread " << std::setw(3) << thrd_id;
      message(t, os.str(), msg, duration_seconds);
    }
    
    void message(timer& t, const std::string& msg,
		 unsigned int duration_seconds) {
      message(t, "\e[1;95m===MAIN===", msg, duration_seconds);
    }
		 
  }
  namespace json {
    struct timeline {
    public:
      struct content {
	std::string message;
	unsigned int duration;
      };
    private:
      std::string filename;
      verbose::timer t;
      std::map<std::string, std::multimap<double, content> > schedule;
      unsigned int indent;

      std::string tab() {
	return std::string(2*indent, ' ');
      }
      
    public:
      timeline(const std::string& filename) : filename(filename), t(), schedule(), indent(0) {}
      
      void operator()(const std::string& tag, const std::string& message, unsigned int duration_seconds) {
	auto& tl = schedule[tag];
	tl.insert({t(), {message, duration_seconds}});
	verbose::message(t, tag, message, duration_seconds);
      }
      
      void operator()(unsigned int thrd_id, const std::string& msg,
		      unsigned int duration_seconds) {
	std::ostringstream os;
	os << "Thread " << std::setw(3) << thrd_id;
	(*this)(os.str(), msg, duration_seconds);
      }
    
      void operator()(const std::string& msg,
		      unsigned int duration_seconds) {
	(*this)("===MAIN===", msg, duration_seconds);
      }
      
      ~timeline() {
	std::ofstream file(filename);
	file << '{' << std::endl;
	indent++;
	for(auto& [tag, info] : schedule) {
	  file << tab() << '\"' << tag << "\": {" << std::endl;
	  indent++;
	  for(auto& [date, elem] : info) {
	    file << tab() << '\"' << date << "\": {\"msg\": \""  << elem.message << "\", \"duration\": " << elem.duration << '}' << std::endl;
	  }
	  indent--;
	  file << tab() << '}' << std::endl;
	}
	
	indent--;
	file << '}' << std::endl;
	file.close();
      }
    };
  }
}
