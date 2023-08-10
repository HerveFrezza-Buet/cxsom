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
    TIMESTEP_LINE := timestep TIMESTEP TIMESTEP_INFO
    JOB_LINE := job JOB_INFO

    JOB_INFO :=   remove TIMESTEP
                | out-of-tasks OUT_OF_TASK_REASON
		| tasks TASK_LIST

    TIMESTEP_INFO := add VARNAME

    TIMESTEP := S(<timeline>,<at>)
    INSTANCE := I(<timeline>,<varname>,<at>)

    OUT_OF_TASK_REASON :=   no-pending-tasks
                          | none-from-timesteps
			  | none-from-patterns

    TASK_LIST = INSTANCE TASK_LIST_END
    TASK_LIST_END = TASK_LIST | <empty>

    

   */
  class Monitor {
  public:
    enum class OutOfTasksReason : unsigned int {NoPendingTasks, NoneFromTimeSteps, NoneFromPatterns};
  private:

    mutable std::mutex mutex;
    mutable std::ofstream out;

    void sep() const {out << ' ';}
    void eol() const {out << std::endl;}
    void tag(const std::string& t) const {out << t;}
    void job_header() const {tag("job");}
    void timestep_header(const symbol::TimeStep& ts) const {tag("timestep"); sep(); timestep(ts);}
    void timestep(const symbol::TimeStep& ts) const {out << "S(" << ts.timeline << ',' << ts.at << ')';}
    void varname(const std::string& name) const {tag(name);}
    void varname(const symbol::Instance& instance) const {varname(instance.variable.name);}
    void instance(const symbol::Instance& instance) const {out << "I(" << instance.variable.timeline << ',' << instance.variable.name << ',' <<  instance.at << ')';} 
    
  public:

    Monitor() : out("monitoring.data") {}

    void job_remove_timestep(const symbol::TimeStep& ts) const {
      std::lock_guard<std::mutex> lock(mutex);
      job_header(); sep(); tag("remove"); sep(); timestep(ts); eol();
    }

    void job_out_of_task(OutOfTasksReason why) {
      job_header(); sep(); tag("out-of-tasks"); sep();
      switch(why) {
      case OutOfTasksReason::NoPendingTasks: tag("no-pending-tasks"); break;
      case OutOfTasksReason::NoneFromTimeSteps: tag("none-from-timesteps"); break;
      default: tag("none-from-patterns"); break;
      };
      eol();
    }

    template<typename INSTANCE_IT>
    void job_task_list(INSTANCE_IT it, INSTANCE_IT end) {
      std::lock_guard<std::mutex> lock(mutex);
      job_header(); sep(); tag("tasks");
      while(it != end) {sep(); instance(*(it++));}
      eol();
    }

    void timestep_add_udate(const symbol::TimeStep& ts, const symbol::Instance& res_update) const {
      std::lock_guard<std::mutex> lock(mutex);
      timestep_header(ts); sep(); tag("add"); sep(); varname(res_update); eol();
    }
  };
}
