#pragma once

#include <chrono>
#include <iostream>
#include <iomanip>
#include <string>
#include <syncstream>
#include <thread>
#include <fstream>
#include <vector>
#include <tuple>
#include <sstream>

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
		 double duration_seconds) {
      std::ostringstream os;
      if(duration_seconds > 0) {
	os.precision(3);
	os << " \e[32m(" << duration_seconds << "s)\e[0m";
      }
      std::osyncstream(std::cout) << "\e[0;90m" << t << "\e[0m : "
				  << "\e[1;94m" << tag << "\e[0m : " 
				  << msg
				  << os.str() << std::endl;
      if(duration_seconds > 0)
	std::this_thread::sleep_for(std::chrono::milliseconds((unsigned int)(1000*duration_seconds)));    
    }

    void message(timer& t, unsigned int thrd_id, const std::string& msg,
		 double duration_seconds) {
      std::ostringstream os;
      os << "Thread " << std::setw(3) << thrd_id;
      message(t, os.str(), msg, duration_seconds);
    }
    
    void message(timer& t, const std::string& msg,
		 double duration_seconds) {
      message(t, "\e[1;95m===MAIN===", msg, duration_seconds);
    }
		 
  }
  namespace json {

    using rgb = std::tuple<double, double, double>;
    
    struct timeline {
    private:
      std::ofstream file;
      verbose::timer t;
      
    public:
      timeline(const std::string& filename) : file(filename), t() {}
      
      void operator()(const std::string& tag, const std::string& msg, double duration_seconds, const rgb& color) {
	std::osyncstream(file) << tag << ';' << t() << ';' << msg << ';' << duration_seconds << ';' << std::get<0>(color) << ' ' <<  std::get<1>(color) << ' ' << std::get<2>(color) << std::endl;
	verbose::message(t, tag, msg, duration_seconds);
      }
      
      void operator()(unsigned int thrd_id, const std::string& msg,
		      double duration_seconds,
		      const rgb& color) {
	std::osyncstream(file) << "Thread " << std::setw(3) << thrd_id << ';' << t() << ';' << msg << ';' << duration_seconds << ';' << std::get<0>(color) << ' ' <<  std::get<1>(color) << ' ' << std::get<2>(color) << std::endl;
	verbose::message(t, thrd_id, msg, duration_seconds);
      }
    
      void operator()(const std::string& msg,
		      double duration_seconds,
		      const rgb& color) {
	std::osyncstream(file) << "===MAIN===" << ';' << t() << ';' << msg << ';' << duration_seconds << ';' << std::get<0>(color) << ' ' <<  std::get<1>(color) << ' ' << std::get<2>(color) << std::endl;
	verbose::message(t, msg, duration_seconds);
      }
      
      ~timeline() {}
    };
  }
}
