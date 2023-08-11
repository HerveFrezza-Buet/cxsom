#pragma once

#include <memory>
#include <set>
#include <vector>
#include <list>
#include <array>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <tuple>

#include <cxsomSymbols.hpp>
#include <cxsomVariable.hpp>
#include <cxsomUpdate.hpp>

namespace cxsom {
  namespace timestep {

    enum class Status : char {
      Unbound  = 'u', //!< The timestep handles updates whose in-arg cannot be updated (unbound).
      Blocked  = 'b', //!< The timestep is blocked due to impossible updates.
      Relaxing = 'r', //!< The timestep is under unstable computation.
      Checking = 'c', //!< Every update seem stable, we are checking this.
      Done     = 'd'  //!< The timestep is done, all updates have been made ready and removed.
    };
    
    inline std::ostream& operator<<(std::ostream& os, const Status& s) {
      switch(s) {
      case Status::Unbound  : os << "unbound" ; break;
      case Status::Blocked  : os << "blocked" ; break;
      case Status::Relaxing : os << "relaxing"; break;
      case Status::Checking : os << "checking"; break;
      case Status::Done     : os << "done"    ; break;
      }
      return os;
    }

    inline std::string to_string(Status s) {
      std::ostringstream ostr;
      ostr << s;
      return ostr.str();
    }
    
    struct content {
      update::ref init;
      update::ref usual;
      bool no_processing = true;
      void init_done() {init = nullptr;}
      bool has_init() const {return init != nullptr;}
      update::ref operator()() const {if(init) return init; return usual;}
      content()                          = delete;
      content(const content&)            = default;
      content& operator=(const content&) = default;
      content(update::ref usual) : init(), usual(usual), no_processing(true) {usual->is_init = false;}
      content(update::ref init, update::ref usual) : init(init), usual(usual), no_processing(true) {init->is_init = true; usual->is_init = false;}
      operator bool() const {return (bool)usual;}
      const std::string& varname() const {return usual->result.who.variable.name;}
      bool operator==(const std::string& name) const {return varname() == name;}
    };

    std::ostream& operator<<(std::ostream& os, const content& c) {
      if(c.has_init())
	os << *(c.init) << ", ";
      os << *(c.usual);
      return os;
    }
    
    class Instance;
    using  ref           = std::shared_ptr<Instance>;
    using wref           = std::weak_ptr<Instance>;
    using update_handler = std::list<content>::iterator;

    
    
#define cxsomTIME_STEP_NB_QUEUES 5
    enum class Queue : unsigned int {
      Unstable   = 0, //!< Updates whose status need to be known.
      Impossible = 1, //!< Updates that have been detected as impossible.
      Stable     = 2, //!< Updates that have been seen stable once are here.
      Confirmed  = 3, //!< Updates whose stability is confirmed.
      New        = 4  //!< The updates newly added to the queue. They stay here until all unbound variables in their arguments (in-arguments) are ready.
    };

    inline std::ostream& operator<<(std::ostream& os, const Queue& s) {
      switch(s) {
      case Queue::Unstable   : os << "unstable"  ; break;
      case Queue::Impossible : os << "impossible"; break;
      case Queue::Stable     : os << "stable"    ; break;
      case Queue::Confirmed  : os << "confirmed" ; break;
      case Queue::New        : os << "new" ; break;
      };
      return os;
    }
    
    inline std::string to_string(Queue q) {
      std::ostringstream ostr;
      ostr << q;
      return ostr.str();
    }

    struct Task {
      ref                           step;
      content                       update;
      Queue                         source;
      update::Status                report;
      Task(ref step, content& update, Queue source);
      Task(const Task&)            = default;
      Task& operator=(const Task&) = default;
      template<typename Fct1, typename Fct2> void operator()(const Fct1&, const Fct2&);
    };
    std::ostream& operator<<(std::ostream& os, const Task& i);
    

    struct UnboundManager {
      std::set<symbol::Variable> unbounds_ready;

      /**
       * @returns True is there are unbound busy variables.
       */
      bool busy_unbounds_found(std::list<content>& new_queue,
			       std::list<content>& unstable_queue,
			       const std::set<symbol::Variable>& updatable_variables) {
#ifdef cxsomLOG
	{
	  logger->msg("");
	  std::ostringstream ostr;
	  if(new_queue.size() > 0)
	    ostr << "Checking for unbound DIs (from the " << new_queue.size() << " updates in the 'new' queue)";
	  else
	    ostr << "Checking for unbound DIs... the 'new' queue is empty ! No unbounds are blocking.";
	  logger->msg(ostr.str());
	  logger->push();
	}
#endif
	bool unbound_busy = false;
	for(auto new_it = new_queue.begin(); new_it != new_queue.end(); /* No increment here */) {
	  auto& u_bar = *new_it;
	  bool that_update_has_unbound_busy = false;
	  for(auto u : {u_bar.init, u_bar.usual})
	    if(u) {
#ifdef cxsomLOG
	      {
		std::ostringstream ostr;
		ostr << "Checking unbounds in update:   " << *u;
		logger->msg(ostr.str());
		logger->push();
	      }
#endif
	      for(auto& arg : u->args_in) {
#ifdef cxsomLOG
		{
		  std::ostringstream ostr;
		  ostr << "Checking if argument " << arg << " is unbound and busy";
		  logger->msg(ostr.str());
		}
#endif
		if(updatable_variables.find(arg.who.variable) == updatable_variables.end()  // Variable is not updated...
		   && unbounds_ready.find(arg.who.variable) == unbounds_ready.end()) { // and not already known as ready.
		
#ifdef cxsomLOG
		  {
		    std::ostringstream ostr;
		    ostr << arg.who.variable << " is unbound... is it ready or busy ?";
		    logger->msg(ostr.str());
		  }
#endif

		  // Let us check the status of arg.
		  data::Availability arg_status;
		  arg.what->get([&arg_status](auto status, auto, auto&) {arg_status = status;});
		  
		  switch(arg_status) {
		  case data::Availability::Busy:	
#ifdef cxsomLOG
		    logger->_msg("... it is busy, the timestep is to be blocked by unbounds.");
#endif
		    unbound_busy = true;
		    that_update_has_unbound_busy = true;
		    break;
		  case data::Availability::Ready:	
#ifdef cxsomLOG
		    logger->_msg("... it is ready.");
#endif
		    unbounds_ready.insert(arg.who.variable);
		    break;
		  default:
		    break;
		  }
		}
		
#ifdef cxsomLOG
		else {
		  std::ostringstream ostr;
		  ostr << arg.who.variable << " is bound.";
		  logger->msg(ostr.str());
		}
#endif

		
	      }
	    }

	  if(that_update_has_unbound_busy) {
#ifdef cxsomLOG
	    logger->msg("The update has busy unbounds, it is kept in the 'new' queue.");
#endif
	    new_it++;
	  }
	  else {
#ifdef cxsomLOG
	    logger->msg("The update has no busy unbounds, it is moves to the 'unstable' queue.");
#endif
	    unstable_queue.push_back(u_bar);
	    new_it = new_queue.erase(new_it);
	  }
	  
#ifdef cxsomLOG
	  logger->pop();
#endif
	  
	}
#ifdef cxsomLOG
	{
	  std::ostringstream ostr;
	  if(unbound_busy)
	    ostr << "Conclusion: We have found unbound busy variables.";
	  else
	    ostr << "Conclusion: We have found no blocking unbound variables.";
	  logger->msg(ostr.str());
	  logger->pop();
	  logger->msg("");
	}
#endif
	return unbound_busy;
      }
    };
    
    class Instance {
    private:
      friend std::ostream& operator<<(std::ostream& os, const Instance& i);

      Status status = Status::Relaxing;
      symbol::TimeStep who;
      
      
      std::set<symbol::TimeStep> blockers;     // The time step that current time step need to be ready.
      std::vector<symbol::TimeStep> new_blockers; // The blockers that have not been reported to an external manager.
      std::set<symbol::Variable> variables; // These are the variables handled in this time step.

      std::array<std::list<content>, cxsomTIME_STEP_NB_QUEUES> queues;
      UnboundManager unbound_manager;



      void move_queue_content(Queue from, Queue to) {
	std::copy(queues[static_cast<unsigned int>(from)].begin(),
		  queues[static_cast<unsigned int>(from)].end(),
		  std::back_inserter(queues[static_cast<unsigned int>(to)]));
	queues[static_cast<unsigned int>(from)].clear();
      }

      // This may block when transitting to Status::Done.

      template<typename NotifyFunc>
      Status update_status(const NotifyFunc& notify_done) {
#ifdef cxsomLOG
	std::ostringstream ostr;
	ostr << "Timestep " << who << " status update : from " << status;
#endif

	if(unbound_manager.busy_unbounds_found(queues[static_cast<unsigned int>(Queue::New)],
					       queues[static_cast<unsigned int>(Queue::Unstable)],
					       variables)) {
	  status = Status::Unbound;
#ifdef cxsomLOG
	  ostr << " to " << status << '.';
	  logger->msg(ostr.str());
#endif
#ifdef cxsomMONITOR
	  notify_update_to_monitor(Monitor::TimeStepUpdateReason::Notification);
#endif
	  return status;
	}
	
	if(queues[static_cast<unsigned int>(Queue::Impossible)].size() > 0) {
	  // This is ok since all updates here are no_processing==true;
	  move_queue_content(Queue::Confirmed, Queue::Stable);
	  if(blockers.empty()) {
	    // This is ok since all updates here are no_processing==true;
	    move_queue_content(Queue::Impossible, Queue::Unstable);
	    status = Status::Relaxing;
	  }
	  else
	    status = Status::Blocked;
#ifdef cxsomLOG
	  ostr << " to " << status << '.';
	  logger->msg(ostr.str());
#endif
#ifdef cxsomMONITOR
	  notify_update_to_monitor(Monitor::TimeStepUpdateReason::Notification);
#endif
	  return status;
	}

	if(queues[static_cast<unsigned int>(Queue::Unstable)].size() > 0) {
	  // This is ok since all updates here are no_processing==true;
	  move_queue_content(Queue::Confirmed, Queue::Stable);
	  status = Status::Relaxing;
#ifdef cxsomLOG
	  ostr << " to " << status << '.';
	  logger->msg(ostr.str());
#endif
#ifdef cxsomMONITOR
	  notify_update_to_monitor(Monitor::TimeStepUpdateReason::Notification);
#endif
	  return status;
	}
	
	if(queues[static_cast<unsigned int>(Queue::Stable)].size() > 0) {
	  status = Status::Checking;
#ifdef cxsomLOG
	  ostr << " to " << status << '.';
	  logger->msg(ostr.str());
#endif
#ifdef cxsomMONITOR
	  notify_update_to_monitor(Monitor::TimeStepUpdateReason::Notification);
#endif
	  return status;
	}

	// Here, we are done.
	status = Status::Done;
	for(auto& e : queues[static_cast<unsigned int>(Queue::Confirmed)]) e()->set_ready();
	queues[static_cast<unsigned int>(Queue::Confirmed)].clear();
	notify_done(who);
#ifdef cxsomLOG
	ostr << " to " << status << '.';
	logger->msg(ostr.str());
#endif
#ifdef cxsomMONITOR
	notify_update_to_monitor(Monitor::TimeStepUpdateReason::Notification);
#endif
	return status;
      }
      
    public:

      Instance()                           = delete;
      Instance(const Instance&)            = default;
      Instance& operator=(const Instance&) = default;
      Instance(Instance&&)                 = default;
      Instance& operator=(Instance&&)      = default;

      Instance(const symbol::TimeStep& who) : who(who) {
#ifdef cxsomMONITOR
	monitor->timestep_launch(who, to_string(status));
#endif
      }

      void notify_update_to_monitor(Monitor::TimeStepUpdateReason why) {
	
	std::vector<symbol::Instance> new_content;
	for(auto& c : queues[static_cast<unsigned int>(Queue::New)])
	  new_content.push_back(c()->result.who);
	
	std::vector<symbol::Instance> unstable_content;
	for(auto& c : queues[static_cast<unsigned int>(Queue::Unstable)])
	  unstable_content.push_back(c()->result.who);
	
	std::vector<symbol::Instance> impossible_content;
	for(auto& c : queues[static_cast<unsigned int>(Queue::Impossible)])
	  impossible_content.push_back(c()->result.who);
	
	std::vector<symbol::Instance> stable_content;
	for(auto& c : queues[static_cast<unsigned int>(Queue::Stable)])
	  stable_content.push_back(c()->result.who);
	
	std::vector<symbol::Instance> confirmed_content;
	for(auto& c : queues[static_cast<unsigned int>(Queue::Confirmed)])
	  confirmed_content.push_back(c()->result.who);
	
	monitor->timestep_update(who, to_string(status), why,
				 to_string(Queue::New), new_content.begin(), new_content.end(),
				 to_string(Queue::Unstable), unstable_content.begin(), unstable_content.end(),
				 to_string(Queue::Impossible), impossible_content.begin(), impossible_content.end(),
				 to_string(Queue::Stable), stable_content.begin(), stable_content.end(),
				 to_string(Queue::Confirmed), confirmed_content.begin(), confirmed_content.end(),
				 blockers.begin(), blockers.end());
	
      }
      
      operator Status()           const {return status;}
      operator symbol::TimeStep() const {return who;}

      void clear_blocking_info() {
#ifdef cxsomLOG
	std::ostringstream ostr;
	ostr << "Timestep " << who << " clears all blocking information";
	logger->msg("");
	logger->msg(ostr.str());
	logger->push();
#endif
	blockers.clear();
	
#ifdef cxsomLOG
	logger->msg("Touching all arguments of the new and impossible updates.");
	logger->push();
#endif
	for(auto& c : queues[static_cast<unsigned int>(Queue::New)]) c()->sync_arguments_availability();
	for(auto& c : queues[static_cast<unsigned int>(Queue::Impossible)]) c()->sync_arguments_availability();
#ifdef cxsomLOG
	logger->pop();
#endif

#ifdef cxsomLOG
	logger->msg("Impossible updates are moved to the unstable queue.");
	logger->msg("Status is set to relaxing.");
#endif
	
	move_queue_content(Queue::Impossible, Queue::Unstable);
	status = Status::Relaxing;
#ifdef cxsomLOG
	logger->pop();
#endif

#ifdef cxsomMONITOR
	notify_update_to_monitor(Monitor::TimeStepUpdateReason::BlockingInfoCleared);
#endif
      }

      template<typename NotifyFunc>
      void notify_unblock(const symbol::TimeStep& blocker, const NotifyFunc& notify_done) {
#ifdef cxsomLOG
	logger->push();
#endif
	blockers.erase(blocker);
	if(blockers.empty()) {
#ifdef cxsomLOG
	  logger->push();
#endif
	  update_status(notify_done);
#ifdef cxsomLOG
	  logger->pop();
#endif
	}
	
#ifdef cxsomLOG
	{
	  std::ostringstream ostr;
	  ostr << "Timestep " << who << " is not blocked anymore by " << blocker
	       << ". Status is now " << status << '.';
	  logger->msg(ostr.str());
	  logger->pop();
	}
#endif
      }
      
      auto new_blockers_range() const {
	return std::make_pair(new_blockers.begin(), new_blockers.end());
      }

      void acq_new_blockers() {new_blockers.clear();}


      bool has_instance(const symbol::Variable& res_var) {
	return variables.find(res_var) != variables.end();
      }
      
      /**
       * This tests if the instance can be added in the time step. If
       * true, it has to be added just after, with the add method (not
       * +=).
       */
      bool test_add(const symbol::Instance& res_inst) {
	if(res_inst.belongs(who)) 
	  if(auto it = variables.find(res_inst); it == variables.end()) {
	    variables.insert(res_inst);
	    return true;
	  }
	return false;
      }

      /**
       * Adds an update without any test (should be done before by test_add).
       */
      void add(const content& update) {
	queues[static_cast<unsigned int>(Queue::New)].push_back(update);
#ifdef cxsomMONITOR
	notify_update_to_monitor(Monitor::TimeStepUpdateReason::NewUpdate);
#endif
      }

      /** 
       * Trys to add the update. It performs test_add and add if relevant.
       */
      void operator+=(const content& update) {if(test_add(*(update.usual))) add(update);}
     
      // This sets the timestep status to Unbound if unbound DIs are found.
      void check_unbound() {
	if(unbound_manager.busy_unbounds_found(queues[static_cast<unsigned int>(Queue::New)],
					       queues[static_cast<unsigned int>(Queue::Unstable)],
					       variables))
	  status = Status::Unbound;
      }

      /**
       * This search for the update in all queues and removes it.
       */
      void remove_update(const std::string& varname) {
	// As queues may have been flushed into others, when an update
	// comes back from evaluation, we cannot know where it is
	// (with the no_processing==false attribute).

	for(auto& queue: queues) {
	  auto begin = queue.begin();
	  auto end = queue.end();
	  for(auto it = begin; it != end; ++it)
	    if(*it == varname) {
	      queue.erase(it);
	      return;
	    }
	}
	
      }
      
      /**
       * This reports the execution of a task.
       */
      template<typename NotifyFunc>
      Status report(const Task& task, const NotifyFunc& notify_done) {
#ifdef cxsomLOG
	{
	  std::ostringstream ostr;
	  ostr << std::endl << "Reporting task (time step is " << who << ", status = " << status << ")";
	  logger->msg(ostr.str());
	  logger->push();
	}
	{
	  std::ostringstream ostr;
	  ostr << task;
	  logger->msg(ostr.str());
	}
#endif
	update::Base::blockers_iter_type begin, end;
	content update = task.update;
	remove_update(update.varname());
	switch(task.report) {
	case update::Status::Impossible:
	  queues[static_cast<unsigned int>(Queue::Impossible)].push_back(update);
	  std::tie(begin, end) = update()->blockers_range();
	  std::copy(begin, end, std::inserter(blockers, blockers.end()));
	  std::copy(begin, end, std::back_inserter(new_blockers));
#ifdef cxsomLOG
	  logger->msg("Putting in the \"impossible\" queue.");
#endif
	  break;
	case update::Status::UpToDate:
	  if(update.has_init()) {
	    // This should never happen, since updates never return UpToDate for inits.
	    update.init_done();
	    queues[static_cast<unsigned int>(Queue::Unstable)].push_back(update);
#ifdef cxsomLOG
	    logger->msg("Updtodate, but init, so putting in \"unstable\" queue.");
#endif
	  }
	  else {
	    if(status == Status::Checking && task.source == Queue::Stable) {
	      queues[static_cast<unsigned int>(Queue::Confirmed)].push_back(update);
#ifdef cxsomLOG
	      logger->msg("Uptodate, checking mode, and was in \"stable\" queue. Putting in \"confirmed\" queue.");
#endif
	    }
	    else {
	      queues[static_cast<unsigned int>(Queue::Stable   )].push_back(update);
#ifdef cxsomLOG
	      logger->msg("Uptodate, needs to be confirmed. Putting in \"stable\" queue.");
#endif
	    }
	  }
	  break;
	case update::Status::Updated:
	  update.init_done();
	  queues[static_cast<unsigned int>(Queue::Unstable)].push_back(update);
#ifdef cxsomLOG
	  logger->msg("Updated. Putting in \"unstable\" queue.");
#endif
	  break;
	case update::Status::Done:
	  if(update.has_init()) {
	    // This should never occur, since execution of init updates never returns Done.
	    update.init_done();
	    queues[static_cast<unsigned int>(Queue::Unstable)].push_back(update);
#ifdef cxsomLOG
	    logger->msg("Done, but it was an init. Putting in \"unstable\" queue.");
#endif
	  }
#ifdef cxsomLOG
	  else {
	    logger->msg("Done (it was not an init). Not putting in any queue.");
	  }
#endif
	  break;
	case update::Status::None:
	  throw std::runtime_error("cxsom::timestep::Instance::operator<= : None update status should never occur here. report bug.");
	  break;
	}

	
#ifdef cxsomLOG
	auto res = update_status(notify_done);
	{
	  std::ostringstream ostr;
	  ostr << "End of reporting, returning " << res << '.';
	  logger->msg(ostr.str());
	  logger->pop();
	}
	return res;
#else
	return update_status(notify_done);
#endif
      }

      /**
       * @param that The share pointer handling this.
       * @returns true if tasks have been actually added.
       */
      template<typename TaksOutputIt>
      bool get_jobs(ref that, TaksOutputIt out) {
	bool res = false;
	std::list<content>::iterator end;

	this->check_unbound();
	
#ifdef cxsomLOG
	{
	  std::ostringstream ostr;
	  ostr << "Time step " << who << " (status = " << status << ") is asked for new jobs.";
	  logger->msg(ostr.str());
	  logger->push();
	}
#endif
	switch(status) {
	case Status::Relaxing :
	  end = queues[static_cast<unsigned int>(Queue::Unstable)].end();
	  for(auto it = queues[static_cast<unsigned int>(Queue::Unstable)].begin(); it != end; ++it)
	    if(it->no_processing) {
	      res = true;
	      *(out++) = {that, *it, Queue::Unstable};
	    }
	  end = queues[static_cast<unsigned int>(Queue::Stable)].end();
	  for(auto it = queues[static_cast<unsigned int>(Queue::Stable)].begin(); it != end; ++it)
	    if(it->no_processing) {
	      res = true;
	      *(out++) = {that, *it, Queue::Stable};
	    }
#ifdef cxsomLOG
	  if(res) logger->msg("found some jobs in \"unstable\" or \"stable\" queue.");
	  else    logger->msg("found no jobs in \"unstable\" nor in \"stable\" queue.");
	  logger->pop();
#endif
	  return res;
	  break;
	case Status::Checking :
	  end = queues[static_cast<unsigned int>(Queue::Stable)].end();
	  for(auto it = queues[static_cast<unsigned int>(Queue::Stable)].begin(); it != end; ++it)
	    if(it->no_processing) {
	      res = true;
	      *(out++) = {that, *it, Queue::Stable};
	    }
#ifdef cxsomLOG
	  if(res) logger->msg("found some jobs in \"stable\" queue.");
	  else    logger->msg("found no jobs in \"stable\" queue.");
	  logger->pop();
#endif
	  return res;
	  break;
	case Status::Unbound :
#ifdef cxsomLOG
	  logger->msg("no jobs, timestep has unbound data instances.");
	  logger->pop();
#endif
	  return false ; break;
	case Status::Blocked :
#ifdef cxsomLOG
	  logger->msg("no jobs, timestep is blocked.");
	  logger->pop();
#endif
	  return false ; break;
	case Status::Done :
#ifdef cxsomLOG
	  logger->msg("no jobs, timestep is done.");
	  logger->pop();
#endif
	  return false    ; break;
	}
	return false; // the thread should never go here.
      }
    };

    inline ref make(const symbol::TimeStep& who) {
      return std::make_shared<Instance>(who);
    }
    
    inline ref make(const std::string& timeline, unsigned int at) {
      return std::make_shared<Instance>(symbol::TimeStep(timeline, at));
    }
      
    inline Task::Task(ref step, content& update, Queue source)
      : step(step), update(update), source(source), report(update::Status::None) {
      update.no_processing = false;
    }
    
    template<typename Fct1, typename Fct2>
    void Task::operator()(const Fct1& notify_blocked, const Fct2& notify_done) {
      report = (*(update()))();
      step->report(*this, notify_done);
      if(report == update::Status::Impossible) notify_blocked(step);
    }
    
    inline std::ostream& operator<<(std::ostream& os, const Task& task) {
      if(task.update)
	os << (*(task.update())) << std::endl;
      else
	os << "<executed> : no more information" << std::endl;
      os << "  src = " << task.source << ", report = " << task.report;
      return os;
    }
    
    inline std::ostream& operator<<(std::ostream& os, const Instance& i) {
      os << i.who << " | status = " << i.status << std::endl
	 << "  blockers:";
      for(auto b : i.blockers) os << ' ' << b;
      os << std::endl;
      os << "  queues:" << std::endl
	 << "    " << std::setw(10) << std::left << Queue::New        << " = " << i.queues[static_cast<unsigned int>(Queue::New)].size()        << std::endl
	 << "    " << std::setw(10) << std::left << Queue::Unstable   << " = " << i.queues[static_cast<unsigned int>(Queue::Unstable)].size()   << std::endl
	 << "    " << std::setw(10) << std::left << Queue::Impossible << " = " << i.queues[static_cast<unsigned int>(Queue::Impossible)].size() << std::endl
	 << "    " << std::setw(10) << std::left << Queue::Stable     << " = " << i.queues[static_cast<unsigned int>(Queue::Stable)].size()     << std::endl
	 << "    " << std::setw(10) << std::left << Queue::Confirmed  << " = " << i.queues[static_cast<unsigned int>(Queue::Confirmed)].size();
      return os;
    }

  }
}

