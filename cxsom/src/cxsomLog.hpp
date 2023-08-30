#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <iterator>
#include <set>

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

    INFO_LINE := JOB_LINE | TIMESTEP_LINE | CHECKPOINT

    CHECKPOINT := # _ <number>
    TIMESTEP_LINE := timestep _ TIMESTEP _ TIMESTEP_INFO
    JOB_LINE := job _ JOB_INFO

    JOB_INFO :=   out-of-tasks _ OUT_OF_TASK_REASON
		| tasks _ TASK_LIST
		| execute INSTANCE

    TIMESTEP_INFO :=   launch _ TIMESTEP_STATUS
                     | terminated _ TIMESTEP_TERMINATED_REASON
                     | add _ VARNAME
		     | update _ TIMESTEP_UPDATE_REASON _ TIMESTEP_STATUS _ TIMESTEP_QUEUES _ TIMESTEP_BLOCKERS _ TIMESTEP_UNBOUNDS
		     | report _ VARNAME _ UPDATE_STATUS
    

    TIMESTEP := S(<timeline>,<at>)
    VARNAME := <varname>
    INSTANCE := I(<timeline>,<varname>,<at>)
    TIMESTEP_STATUS := [ TS_STATUS ]
    TS_STATUS := unbound | blocked | relaxing | checking | done
    UPDATE_STATUS := [ UPDT_STATUS ]
    UPDT_STATUS := impossible | up-to-date | updated | done | none

    OUT_OF_TASK_REASON := no-pending-tasks | none-from-timesteps | none-from-patterns  
    TIMESTEP_UPDATE_REASON := notification | new-update | blocking-info-cleared | unbound
    TIMESTEP_TERMINATED_REASON := done | clear-all

    TASK_LIST := INSTANCE TASK_LIST_END
    TASK_LIST_END := _ TASK_LIST | <empty>
    
    TIMESTEP_QUEUES := new QUEUE_CONTENT unstable QUEUE_CONTENT impossible QUEUE_CONTENT stable QUEUE_CONTENT confirmed QUEUE_CONTENT

    QUEUE_CONTENT := <nb-elems> QUEUE_ELEMS

    QUEUE_ELEMS :=   _ VARNAME QUEUE_ELEMS
                   | <empty>

    TIMESTEP_BLOCKERS := blockers <nb-elems> BLOCKERS_ELEMS
    BLOCKERS_ELEMS :=   _ TIMESTEP  BLOCKERS_ELEMS
                      | <empty>

    TIMESTEP_UNBOUNDS := unbounds <nb-elems> UNBOUNDS_ELEMS
    UNBOUNDS_ELEMS :=   _ VARNAME  UNBOUNDS_ELEMS
                      | <empty>
    

   */
  class Monitor {
  public:
    enum class OutOfTasksReason : unsigned int {NoPendingTasks, NoneFromTimeSteps, NoneFromPatterns};
    enum class TimeStepUpdateReason : unsigned int {Notification, NewUpdate, BlockingInfoCleared, Unbound};
    enum class TimeStepTerminatedReason : unsigned int {Done, ClearAll};
  private:

    mutable std::mutex mutex;
    mutable std::ofstream out;
    mutable unsigned int checkpoint_id = 0;

    void sep() const {out << ' ';}
    void eol() const {out << std::endl;}
    void tag(const std::string& t) const {out << t;}
    void nb(int n) const {out << n;}
    void job_header() const {tag("job");}
    void timestep_header(const symbol::TimeStep& ts) const {tag("timestep"); sep(); timestep(ts);}
    void timestep(const symbol::TimeStep& ts) const {out << "S(" << ts.timeline << ',' << ts.at << ')';}
    void timestep_status(const std::string& status) const {out << '[' << status << ']';}
    void update_status(const std::string& status) const {timestep_status(status);}
    void varname(const std::string& name) const {tag(name);}
    void varname(const symbol::Instance& instance) const {varname(instance.variable.name);}
    void instance(const symbol::Instance& instance) const {out << "I(" << instance.variable.timeline << ',' << instance.variable.name << ',' <<  instance.at << ')';}
    

    std::set<symbol::Variable> unbound_variables;
    
  public:

    Monitor() : out("monitoring.data") {}
    
    void checkpoint() const {tag("#"); sep(); nb(checkpoint_id++); eol();}


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

    void job_execute(const symbol::Instance& res_update) const {
      job_header(); sep(); tag("execute"); sep(); instance(res_update); eol();
    }

    void timestep_add_update(const symbol::TimeStep& ts, const symbol::Instance& res_update) const {
      std::lock_guard<std::mutex> lock(mutex);
      timestep_header(ts); sep(); tag("add"); sep(); varname(res_update); eol();
    }

    void timestep_launch(const symbol::TimeStep& ts, const std::string& status) const {
      timestep_header(ts); sep(); tag("launch"); sep(); timestep_status(status); eol();
    }

    void timestep_terminated(const symbol::TimeStep& ts, TimeStepTerminatedReason why) const {
      timestep_header(ts); sep(); tag("terminated"); sep();
      switch(why) {
      case TimeStepTerminatedReason::Done: tag("done"); break;
      case TimeStepTerminatedReason::ClearAll:
      default: tag("done"); break;
      }
      eol();
    }

    void clear_unbounds() {unbound_variables.clear();}
    void unbound_variable_found(const symbol::Variable& v) {unbound_variables.insert(v);}
    
    void timestep_update_report(const symbol::TimeStep& ts, const symbol::Instance& res_update, const std::string& status) {
      timestep_header(ts); sep(); tag("report"); sep(); varname(res_update); sep(); update_status(status); eol();
    }
    template<typename INSTANCE_IT, typename TIMESTEP_IT>
    void timestep_update(const symbol::TimeStep& ts, std::string status, TimeStepUpdateReason why,
			 const std::string& new_name, INSTANCE_IT new_begin, INSTANCE_IT new_end,
			 const std::string& unstable_name, INSTANCE_IT unstable_begin, INSTANCE_IT unstable_end,
			 const std::string& impossible_name, INSTANCE_IT impossible_begin, INSTANCE_IT impossible_end,
			 const std::string& stable_name, INSTANCE_IT stable_begin, INSTANCE_IT stable_end,
			 const std::string& confirmed_name, INSTANCE_IT confirmed_begin, INSTANCE_IT confirmed_end,
			 TIMESTEP_IT blockers_begin, TIMESTEP_IT blockers_end) const {
      timestep_header(ts); sep(); tag("update"); sep(); 
      switch(why) {
      case TimeStepUpdateReason::Notification: tag("notification"); break;
      case TimeStepUpdateReason::NewUpdate: tag("new-update"); break;
      case TimeStepUpdateReason::BlockingInfoCleared: tag("blocking-info-cleared"); break;
      case TimeStepUpdateReason::Unbound:
      default: tag("unbound"); break;
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
      
      sep(); tag("blockers"); sep(); nb(std::distance(blockers_begin, blockers_end));
      while(blockers_begin != blockers_end) {sep(); timestep(*(blockers_begin++));}
      
      sep(); tag("unbounds"); sep(); nb(unbound_variables.size());
      for(auto& var : unbound_variables) {sep(); varname(var.name);}

      eol();
    }
  };
}
