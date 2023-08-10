#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <iterator>

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
    _ is a space caracter

    INFO_LINE := JOB_LINE | TIMESTEP_LINE
    TIMESTEP_LINE := timestep _ TIMESTEP _ TIMESTEP_INFO
    JOB_LINE := job _ JOB_INFO

    JOB_INFO :=   remove _ TIMESTEP
                | out-of-tasks _ OUT_OF_TASK_REASON
		| tasks _ TASK_LIST

    TIMESTEP_INFO :=   launch _ TIME_STEP_STATUS
                     | add _ VARNAME
		     | update _ TIME_STEP_UPDATE_REASON _ TIME_STEP_STATUS _ TIMESTEP_QUEUES
    

    TIMESTEP := S(<timeline>,<at>)
    INSTANCE := I(<timeline>,<varname>,<at>)
    TIME_STEP_STATUS := [ TS_STATUS ]
    TS_STATUS := unbound | blocked | relaxing | checking | done

    OUT_OF_TASK_REASON := no-pending-tasks | none-from-timesteps | none-from-patterns  
    TIME_STEP_UPDATE_REASON := notification | new-update | blocking-info-cleared

    TASK_LIST := INSTANCE TASK_LIST_END
    TASK_LIST_END := _ TASK_LIST | <empty>
    
    TIMESTEP_QUEUES := new QUEUE_CONTENT unstable QUEUE_CONTENT impossible QUEUE_CONTENT stable QUEUE_CONTENT confirmed QUEUE_CONTENT

    QUEUE_CONTENT := <nb-elems> QUEUE_ELEMS

    QUEUE_ELEMS :=   _ VARNAME QUEUE_ELEMS
                   | <empty>
    

   */
  class Monitor {
  public:
    enum class OutOfTasksReason : unsigned int {NoPendingTasks, NoneFromTimeSteps, NoneFromPatterns};
    enum class TimeStepUpdateReason : unsigned int {Notification, NewUpdate, BlockingInfoCleared};
  private:

    mutable std::mutex mutex;
    mutable std::ofstream out;

    void sep() const {out << ' ';}
    void eol() const {out << std::endl;}
    void tag(const std::string& t) const {out << t;}
    void nb(int n) const {out << n;}
    void job_header() const {tag("job");}
    void timestep_header(const symbol::TimeStep& ts) const {tag("timestep"); sep(); timestep(ts);}
    void timestep(const symbol::TimeStep& ts) const {out << "S(" << ts.timeline << ',' << ts.at << ')';}
    void timestep_status(const std::string& status) const {out << '[' << status << ']';}
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
      case OutOfTasksReason::NoneFromPatterns:
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

    void timestep_launch(const symbol::TimeStep& ts, std::string status) const {
      timestep_header(ts); sep(); tag("launch"); sep(); timestep_status(status); eol();
    }

    template<typename INSTANCE_IT>
    void timestep_update(const symbol::TimeStep& ts, std::string status, TimeStepUpdateReason why,
			 const std::string& new_name, INSTANCE_IT new_begin, INSTANCE_IT new_end,
			 const std::string& unstable_name, INSTANCE_IT unstable_begin, INSTANCE_IT unstable_end,
			 const std::string& impossible_name, INSTANCE_IT impossible_begin, INSTANCE_IT impossible_end,
			 const std::string& stable_name, INSTANCE_IT stable_begin, INSTANCE_IT stable_end,
			 const std::string& confirmed_name, INSTANCE_IT confirmed_begin, INSTANCE_IT confirmed_end) const {
      timestep_header(ts); sep(); tag("update"); sep(); 
      switch(why) {
      case TimeStepUpdateReason::Notification: tag("notification"); break;
      case TimeStepUpdateReason::NewUpdate: tag("new-update"); break;
      case TimeStepUpdateReason::BlockingInfoCleared:
      default: tag("blocking-info-cleared"); break;
      }
      sep();
      timestep_status(status);
      
      sep(); tag(new_name); sep(); nb(std::distance(new_begin, new_end));
      while(new_begin != new_end) {sep(); varname(*(new_begin++));}
      
      sep(); tag(unstable_name); sep(); nb(std::distance(unstable_begin, unstable_end));
      while(unstable_begin != unstable_end) {sep(); varname(*(unstable_begin++));}
      
      sep(); tag(impossible_name); sep(); nb(std::distance(impossible_begin, impossible_end));
      while(impossible_begin != impossible_end) {sep(); varname(*(impossible_begin++));}
      
      sep(); tag(stable_name); sep(); nb(std::distance(stable_begin, stable_end));
      while(stable_begin != stable_end) {sep(); varname(*(stable_begin++));}
      
      sep(); tag(confirmed_name); sep(); nb(std::distance(confirmed_begin, confirmed_end));
      while(confirmed_begin != confirmed_end) {sep(); varname(*(confirmed_begin++));}

      eol();
    }
  };
}
