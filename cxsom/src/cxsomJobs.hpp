#pragma once

#include <cxsomSymbols.hpp>
#include <cxsomVariable.hpp>
#include <cxsomUpdate.hpp>
#include <cxsomTimeStep.hpp>
#include <cxsomData.hpp>
#include <cxsomOperation.hpp>
#include <cxsomJobRule.hpp>

#include <map>
#include <vector>
#include <string>
#include <optional>
#include <stdexcept>
#include <sstream>
#include <iterator>
#include <deque>
#include <functional>
#include <algorithm>
#include <iomanip>
#include <memory>

#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>

#include <skednet.hpp>


namespace cxsom {

  
  namespace error {
    struct bad_update : public std::logic_error {
      using std::logic_error::logic_error;
    };
  }
  
  namespace jobs {

    class Center {
    private:

      std::mt19937 gen;
      UpdateFactory update_factory;
      TypeChecker type_checker;
      data::Center& data_center;
      mutable std::mutex integrity_mutex;

      std::map<symbol::TimeStep, timestep::ref>   timesteps;
      std::deque<timestep::Task>                  tasks;
      std::map<symbol::Variable, pattern::Update> patterns;
      
      std::vector<cxsom::symbol::TimeStep> terminated_ts;
      std::map<cxsom::symbol::TimeStep, std::set<cxsom::timestep::ref>> blockees;

      std::atomic<bool>               interaction_ongoing;
      mutable std::mutex              job_mutex;
      mutable std::condition_variable pending_jobs;
      std::atomic<unsigned int>       nb_ongoing_processes;

      std::vector<type::ref> arg_types_tmp;
      
      void clean_timesteps() {
	std::map<symbol::TimeStep, timestep::ref>::iterator it;
	for(it = timesteps.begin(); it != timesteps.end();) {
	  auto curr = it++;
	  if(timestep::Status(*(curr->second)) == timestep::Status::Done) {
#ifdef cxsomLOG
	    std::ostringstream ostr;
	    ostr << "Removing timestep " << symbol::TimeStep(*(curr->second)) << ", since its status is done.";
	    logger->msg(ostr.str());
#endif
#ifdef cxsomMONITOR
	    monitor->timestep_terminated(curr->first, Monitor::TimeStepTerminatedReason::Done);
#endif
	    timesteps.erase(curr);
	  }
	  else {
#ifdef cxsomLOG
	    std::ostringstream ostr;
	    ostr << "Keeping  timestep " << symbol::TimeStep(*(curr->second)) << ", since its status is " << timestep::Status(*(curr->second)) << '.';
	    logger->msg(ostr.str());
#endif
	  }
	}
      }
      
      void integrity_notify_blocked(cxsom::timestep::ref me) {
	std::lock_guard<std::mutex> lock(integrity_mutex);
	auto [begin, end] = me->new_blockers_range();
	for(auto it = begin; it != end; ++it) blockees[*it].insert(me);
	me->acq_new_blockers();
      }
      
      void notify_done(const cxsom::symbol::TimeStep& me) {
	terminated_ts.push_back(me);
	if(auto it = blockees.find(me); it != blockees.end()) {
	  for(auto ts : it->second) ts->notify_unblock(me, [this](const auto& me){this->notify_done(me);});
	}
      }
      
      void integrity_notify_done(const cxsom::symbol::TimeStep& me) {
	std::lock_guard<std::mutex> lock(integrity_mutex);
	notify_done(me);
      }


      timestep::ref check_and_get(const symbol::TimeStep& ts) {
	if(auto it = timesteps.find(ts); it != timesteps.end())
	  return it->second;
	auto res = cxsom::timestep::make(ts);
	timesteps[ts] = res;
	return res;
      }

      void add_update(const Update& updt, timestep::ref ts) {
#ifdef cxsomLOG
	logger->msg("Add update:");
	{
	  std::ostringstream ostr;
	  ostr << updt;
	  logger->_msg(ostr.str());
	}
#endif
#ifdef cxsomMONITOR
	monitor->timestep_add_update(*ts, updt.res);
#endif
	auto& usual = updt.usual; 
	update::arg arg_res {updt.res, data_center.type_of(updt.res)};
	std::vector<update::arg> usual_args;
	auto usual_out = std::back_inserter(usual_args);
	for(auto& a : usual.args) *(usual_out++) = {a, data_center.type_of(a)};
	if(updt.init) {
	  auto& init = *(updt.init);
	  std::vector<update::arg> init_args;
	  auto init_out = std::back_inserter(init_args);
	  for(auto& a : init.args) *(init_out++) = {a, data_center.type_of(a)};
	  *ts += {
	    update_factory  (data_center, init.op,  arg_res,  init_args,  init.params, gen()),
	      update_factory(data_center, usual.op, arg_res, usual_args, usual.params, gen())
	      };
	}
	else
	  *ts += {update_factory(data_center, usual.op, arg_res, usual_args, usual.params, gen())};
      }

      
      
      void unprotected_realize_patterns() {
#ifdef cxsomLOG
	logger->msg("We have to realize patterns");
	logger->push();
#endif
	std::map<std::string, std::size_t> min_update_time_in_timeline;
#ifdef cxsomLOG
	logger->msg("Finding the earliest updates in timelines");
	logger->push();
#endif
	for(auto& kv : patterns) {
	  const auto& timeline = kv.first.timeline;
	  auto at = data_center.history_length(kv.second.res);
#ifdef cxsomLOG
	  std::ostringstream ostr1;
	  ostr1 << "Considering pattern updating " << kv.second.res << " in timeline " << kv.first.timeline << " : history length = " << at;
	  logger->msg(ostr1.str());
	  logger->push();
#endif
	  if(at <= kv.second.walltime) {
	    if(auto iter = min_update_time_in_timeline.find(timeline); iter == min_update_time_in_timeline.end())
	      min_update_time_in_timeline[timeline] = at;
	    else
	      iter->second = std::min(iter->second, at);
#ifdef cxsomLOG	    
	    std::ostringstream ostr2; 
	    ostr2 << "since history length (" << at << ") <= walltime (" << pattern::walltime_to_string(kv.second.walltime)
		  << "), earliest update time is re-evaluated : result is finally " << min_update_time_in_timeline[timeline] << '.';
	    logger->msg(ostr2.str());
#endif
	  }
	  else {
#ifdef cxsomLOG
	    std::ostringstream ostr2;
	    ostr2 << "History length (" << at << ") > walltime (" << pattern::walltime_to_string(kv.second.walltime) << ')';
	    logger->msg(ostr2.str());
#endif
#ifdef cxsomMONITOR
	    monitor->timestep_dont_add_update(kv.first, at);
#endif
	  }

	  
#ifdef cxsomLOG
	  logger->pop();
#endif
	}
#ifdef cxsomLOG
	  logger->pop();
#endif

	if(min_update_time_in_timeline.size() == 0) {
#ifdef cxsomLOG
	  logger->msg("No feasible update has been found from current patterns");
	  logger->pop();
#endif
	  return;
	}
	
#ifdef cxsomLOG
	logger->msg("Earliest updates in timelines have been found:");
	logger->push();
	for(auto& kv : min_update_time_in_timeline) {
	  std::ostringstream ostr;
	  ostr << "timeline(" << kv.first << ")@" << kv.second;
	  logger->msg(ostr.str());
	}
	logger->pop();
#endif
	

	  
#ifdef cxsomLOG
	logger->push();
#endif
	for(auto& kv : patterns)
	  if(auto iter = min_update_time_in_timeline.find(kv.first.timeline);
	     iter != min_update_time_in_timeline.end())
	    unprotected_realize(kv.second, iter->second);
#ifdef cxsomLOG
	logger->pop();
#endif
	  
#ifdef cxsomLOG
	logger->pop();
#endif
      }
      
      void unprotected_realize(const pattern::Update& updt, std::size_t min_at) {
#ifdef cxsomLOG
	std::size_t indent = logger->indentation;
#endif
	try {
	  auto at = data_center.history_length(updt.res);
#ifdef cxsomLOG
	  {
	    std::ostringstream ostr;
	    
	    ostr << "considering pattern " << updt.res << " at " << at << " : walltime = ";
	    if(updt.walltime == std::numeric_limits<unsigned int>::max())
	      ostr << "infinity";
	    else
	      ostr << updt.walltime;
	    if(at > updt.walltime)
	      ostr << "... ignored since walltime is over.";
	    else if(at != min_at)
	      ostr << "... ignored since previous timesteps need to be done in the timeline.";
	    logger->msg(ostr.str());
	  }											      
#endif
	  
	  if(at <= updt.walltime && at == min_at) {
	    auto         res  = symbol::Instance(updt.res, at);
	    auto         ts   = check_and_get(res);
#ifdef cxsomLOG
	    logger->push();
	    bool hs = ts->has_instance(res);
	    {
	      std::ostringstream ostr;
	      ostr << "timestep " << symbol::TimeStep(*ts) << " has instance "
		   << res << " ? " << std::boolalpha
		   << hs << '.';
	      logger->msg(ostr.str());
	    }
	    if(hs)
	      logger->_msg("  ... ignoring patten.");
	    else {
	      logger->push();
	      std::ostringstream ostr;
	      logger->msg("Instanciating update from pattern (may raise an exception)");
	      auto u = pattern::at(updt, at);
	      ostr << "inserting update from pattern: " << std::endl
		   << u;
	      logger->msg(ostr.str());
	      add_update(u, ts);
	      logger->pop();
	    }
	    logger->pop();
#else
	    if(!(ts->has_instance(res))) add_update(pattern::at(updt, at), ts);
#endif
	  }
	}
	catch(const error::negative_time& e) {
#ifdef cxsomLOG
	  logger->indentation = indent;
	  logger->_msg(std::string("Exception caught : ") + e.what() + " update addition aborted.");
#endif
	}
      }

      template<typename UPDT>
      void type_checking(const UPDT& updt) {
	auto res_type = data_center.type_of(updt.res);
	if(updt.init) {
	  arg_types_tmp.clear();
	  auto out = std::back_inserter(arg_types_tmp);
	  for(auto arg : updt.init->args) *(out++) = data_center.type_of(arg);
	  type_checker(updt.init->op, res_type, arg_types_tmp);
	}
	arg_types_tmp.clear();
	auto out = std::back_inserter(arg_types_tmp);
	for(auto arg : updt.usual.args) *(out++) = data_center.type_of(arg);
	type_checker(updt.usual.op, res_type, arg_types_tmp);
      }


      std::shared_ptr<sked::net::scope::xrsw::write_explicit> xrsw_writer;
      bool flushing_tasks_in_progress;
      void on_start_flushing_tasks() {
	if(xrsw_writer) xrsw_writer->enter();
      }
      
      void on_end_flushing_tasks() {
	if(xrsw_writer) xrsw_writer->leave();
      }
      
    public:
      
      template<typename RANDOM_ENGINE>
      Center(RANDOM_ENGINE& rd, const UpdateFactory& update_factory, const TypeChecker& type_checker, data::Center& data_center,
	     std::shared_ptr<sked::net::scope::xrsw::write_explicit> xrsw_writer)
	: gen(rd()),
	  update_factory(update_factory),
	  type_checker(type_checker),
	  data_center(data_center),
	  integrity_mutex(),
	  timesteps(),
	  tasks(),
	  patterns(),
	  terminated_ts(),
	  blockees(),
	  interaction_ongoing(false),
	  job_mutex(),
	  pending_jobs(),
	  nb_ongoing_processes(0),
	  xrsw_writer(xrsw_writer),
	  flushing_tasks_in_progress(false)
      {}
      Center()                         = delete;
      Center(const Center&)            = default;
      Center& operator=(const Center&) = default;
      Center(Center&&)                 = default;
      Center& operator=(Center&&)      = default;
      
      void operator+=(const pattern::Update& updt) {
	std::lock_guard<std::mutex> lock(integrity_mutex);
	type_checking(updt);
	if(auto it = patterns.find(updt.res); it == patterns.end())
	  patterns.try_emplace(updt.res, updt);
	else
	  it->second = updt;
#ifdef cxsomLOG
	logger->msg("Add pattern:");
	{
	  std::ostringstream ostr;
	  ostr << updt;
	  logger->_msg(ostr.str());
	}
#endif
      }
      
      void operator+=(const Update& updt) {
	std::lock_guard<std::mutex> lock(integrity_mutex);
	if(data::Availability(*(data_center[updt.res])) == data::Availability::Ready)
	  return;
	type_checking(updt);
	add_update(updt, check_and_get(updt.res));
      }

      void clear() {
	std::lock_guard<std::mutex> lock(integrity_mutex);
#ifdef cxsomLOG
	logger->msg("clearing everything !.");
#endif
#ifdef cxsomMONITOR
	for(const auto& [ts, ts_ref] : timesteps)
	  monitor->timestep_terminated(ts, Monitor::TimeStepTerminatedReason::ClearAll);
#endif
	timesteps.clear();
	tasks.clear();
	patterns.clear();
	terminated_ts.clear();
	blockees.clear();
	arg_types_tmp.clear();
      }

      /**
       * This removes information about blocking (for ping). Every
       * impossible update become unstable, in order to be re-tested.
       */
      void clear_blocking_info() {
	std::lock_guard<std::mutex> lock(integrity_mutex);
#ifdef cxsomLOG
	logger->msg("");
	logger->msg("clear_blocking_info()... for all timesteps.");
	logger->push();
#endif
	blockees.clear();
	for(auto& kv: timesteps)
	  kv.second->clear_blocking_info();
#ifdef cxsomLOG
	logger->pop();
#endif
      }

      /**
       * We call this in mono-thread mode (locking integrity_mutex) in order to get the next job to do.
       */
      std::function<void ()> get_one() {	
#ifdef cxsomMONITOR
	monitor->checkpoint();
#endif
	
#ifdef cxsomLOG
	logger->msg("");
	logger->msg("get_one()...");
	logger->push();
#endif
	std::lock_guard<std::mutex> lock(integrity_mutex);
#ifdef cxsomLOG
	logger->msg("... mutex passed.");
#endif
	if(tasks.empty()) {
#ifdef cxsomMONITOR
	  monitor->job_out_of_task(Monitor::OutOfTasksReason::NoPendingTasks);
#endif
#ifdef cxsomLOG
	  logger->msg("No more tasks, purging done timesteps and ask new jobs to the remaining ones.");
	  logger->push();
#endif
	  clean_timesteps();
#ifdef cxsomLOG
	  logger->pop();
#endif
	  for(auto& kv : timesteps) {
#ifdef cxsomLOG
	    {
	      std::ostringstream ostr;
	      ostr << "asking jobs to timestep " << kv.first << '.';
	      logger->msg(ostr.str());
	    }
	    {
	      std::ostringstream ostr;
	      ostr << *(kv.second);
	      logger->_msg(ostr.str());
	    }
#endif
	    kv.second->get_jobs(kv.second, std::back_inserter(tasks));
#ifdef cxsomLOG
	    {
	      std::ostringstream ostr;
	      ostr << *(kv.second);
	      logger->_msg(ostr.str());
	    }
	    logger->msg(std::string("tasks size: ") + std::to_string(tasks.size()));
#endif
	  }
#ifdef cxsomLOG
	  {
	    std::ostringstream ostr;
	    ostr << std::endl
		 << "# Tasks";
	    logger->msg(ostr.str());
	  }
	  for(auto& task : tasks) {
	    std::ostringstream ostr;
	    ostr << task;
	    logger->_msg(ostr.str());
	  }
#endif
	}
	if(tasks.empty()) {
#ifdef cxsomMONITOR
	  monitor->job_out_of_task(Monitor::OutOfTasksReason::NoneFromTimeSteps);
#endif
#ifdef cxsomLOG
	  logger->msg("Still no more tasks, realizing patterns.");
	  logger->push();
#endif
	  unprotected_realize_patterns();
	  
#ifdef cxsomLOG
	  logger->pop();
#endif
	  
	  for(auto& kv : timesteps) {
#ifdef cxsomLOG
	    {
	      std::ostringstream ostr;
	      ostr << "asking jobs to timestep " << kv.first << '.';
	      logger->msg(ostr.str());
	    }
	    {
	      std::ostringstream ostr;
	      ostr << *(kv.second);
	      logger->_msg(ostr.str());
	    }
#endif
	    kv.second->get_jobs(kv.second, std::back_inserter(tasks));
#ifdef cxsomLOG
	    {
	      std::ostringstream ostr;
	      ostr << *(kv.second);
	      logger->_msg(ostr.str());
	    }
	    logger->msg(std::string("tasks size: ") + std::to_string(tasks.size()));
#endif
	  }
#ifdef cxsomLOG
	  {
	    std::ostringstream ostr;
	    ostr << std::endl
		 << "# Tasks";
	    logger->msg(ostr.str());
	  }
	  for(auto& task : tasks) {
	    std::ostringstream ostr;
	    ostr << task;
	    logger->_msg(ostr.str());
	  }
#endif
	}
	
	if(tasks.empty())  {
#ifdef cxsomMONITOR
	  monitor->job_out_of_task(Monitor::OutOfTasksReason::NoneFromPatterns);
#endif
#ifdef cxsomLOG
	  logger->msg("There are really no more tasks, no jobs found.");
	  logger->pop();
#endif
	  if(nb_ongoing_processes == 0 and flushing_tasks_in_progress) {
	    flushing_tasks_in_progress = false;
	    on_end_flushing_tasks();
	  }
	  return {};
	}

	++nb_ongoing_processes;
	if(!flushing_tasks_in_progress && nb_ongoing_processes == 1) {
	  flushing_tasks_in_progress = true;
	  on_start_flushing_tasks();
	}

#ifdef cxsomMONITOR
	{
	  std::vector<symbol::Instance> tbd_instances;
	  auto tbd_out = std::back_inserter(tbd_instances);
	  for(auto& task : tasks)
	    *(tbd_out++) = task.update.usual->result.who;
	  monitor->job_task_list(tbd_instances.begin(), tbd_instances.end());
	}
#endif
	
	
	auto task = tasks.front();
#ifdef cxsomLOG
	logger->msg("got task:");
	{
	  std::ostringstream ostr;
	  ostr << task;
	  logger->_msg(ostr.str());
	}
#endif

	
#ifdef cxsomMONITOR
	monitor->job_execute(task.update.usual->result.who);
#endif
	
	tasks.pop_front();
#ifdef cxsomLOG
	logger->pop();
#endif
	return [this, task]() mutable {
		 task([this](auto        me) {this->integrity_notify_blocked(me);},
		      [this](const auto& me) {this->integrity_notify_done(me);   });
#ifdef cxsomLOG
		 logger->msg("status after task execution:");
		 std::ostringstream ostr;
		 ostr << task;
		 logger->_msg(ostr.str());
#endif
	       };

      }

      /**
       * This function is a worker thread.
       */
      void worker_thread() {
	std::unique_lock<std::mutex> lock(job_mutex);
	while(true) {
	  if(interaction_ongoing) pending_jobs.wait(lock);
	  else {
	    if(auto job = get_one(); job) {
	      job();
	      --nb_ongoing_processes;
	    }
	    else pending_jobs.wait(lock);
	  }
	}
      }

      /**
       * This interrupts the runing of new updates so that interaction can be done.
       */
      void interaction_lock()    {interaction_ongoing = true;}

      /**
       * This ends interaction sequence, and notifies worker that some
       * work may be pending. This can be use without a former call of
       * interaction_lock, just for starting the computation.
       */
      void interaction_release() {
	interaction_ongoing = false;
	{
	  std::unique_lock<std::mutex> lock(job_mutex);
	  pending_jobs.notify_all();
	}
      }
    };
  }
}
