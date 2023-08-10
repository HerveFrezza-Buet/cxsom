#include <string>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>
#include <deque>
#include <map>
#include <set>
#include <iterator>


#define cxsomLOG
#include <cxsom-server.hpp>

// This is used in case of some macros are set. You can remove this
// lines otherwise... or keep it.
cxsom::Tick*    cxsom::ticker  = new cxsom::Tick();
cxsom::Log*     cxsom::logger  = new cxsom::Log();
cxsom::Monitor* cxsom::monitor = new cxsom::Monitor();


#include <filesystem>
namespace fs = std::filesystem;

// This does no real computation. It reads arguments only, and stores
// as a result the number of time it has been updated. There is a
// limit in that number (i.e. a dead-line) after which the update is
// considered as non significant, i.e as if the computed value do not
// change from the previous one.
class Dummy : public cxsom::update::Base {
private:
  std::string name;
  unsigned int count = 0;
  unsigned int deadline = 0;
  bool out_ok = false;
  friend std::ostream& operator<<(std::ostream&, const Dummy&);
public:
  using cxsom::update::Base::Base;
  void set_info(const cxsom::symbol::Instance& who, unsigned int deadline) {
    std::ostringstream ostr;
    ostr << who;
    this->name     = ostr.str();
    this->deadline = deadline;
  }

  virtual std::string function_name() const override {
    return "dummy";
  }
  
protected:
  
  virtual void on_computation_start() override {
    cxsom::logger->_msg("computation starts...");
  }
  
  virtual void on_read_out_arg(const cxsom::symbol::Instance& who,           // The instance where the argument is.
			       unsigned int arg_number,                      // Its rank in the update argument list.
			       const cxsom::data::Base&) override {          // The data associated to the instance.
    std::ostringstream ostr;
    ostr << "- #" << arg_number << "(out)=" << who;
    cxsom::logger->_msg(ostr.str());
  }
  
  virtual void on_read_out_arg_aborted() override {
    cxsom::logger->_msg("- out aborted.");
  }
    
  virtual void on_read_in_arg(const cxsom::symbol::Instance& who,
			      unsigned int arg_number,
			      const cxsom::data::Base&) override {
    std::ostringstream ostr;
    ostr << "- #" << arg_number << "(in)=" << who;
    cxsom::logger->_msg(ostr.str());
  }
  
  virtual bool on_write_result(cxsom::data::Base&) override {
    if(count < deadline) ++count;
    cxsom::logger->_msg(std::string("- result = ") + std::to_string(count));
    return count < deadline;
  }
};


std::ostream& operator<<(std::ostream& os, const Dummy& d) {
  os << '\"' << d.name << "\":" << d.count << '/' << d.deadline;
  return os;
}


// This is a handcrafted function for getting tasks to be done.
template<typename TP, typename T, typename B> void flush_tasks(TP& timesteps, T& tasks, B& blockees);

using blockees_map = std::map<cxsom::symbol::TimeStep, std::set<cxsom::timestep::ref>>;

#define DEADLINE        5
#define CACHE_SIZE     10
#define FILE_SIZE    1000
#define KEPT_OPENED  true

int main(int, char**) {

  std::deque<cxsom::timestep::Task> tasks;
  blockees_map blockees; // blockees[ts] are the time steps waiting for ts to be done.
  
  cxsom::data::Center center(fs::current_path() / "tmp");

  // We declare the variables
  center.check({"1", "A"}, cxsom::type::make("Pos1D"), CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
  center.check({"1", "B"}, cxsom::type::make("Pos1D"), CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
  center.check({"2", "A"}, cxsom::type::make("Pos1D"), CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
  center.check({"2", "B"}, cxsom::type::make("Pos1D"), CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
  center.check({"3", "A"}, cxsom::type::make("Pos1D"), CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
  center.check({"3", "B"}, cxsom::type::make("Pos1D"), CACHE_SIZE, FILE_SIZE, KEPT_OPENED);

  std::map<cxsom::symbol::TimeStep, cxsom::timestep::ref> timesteps;
  timesteps[{"1", 0}] = cxsom::timestep::make("1", 0);
  timesteps[{"2", 0}] = cxsom::timestep::make("2", 0);
  timesteps[{"3", 0}] = cxsom::timestep::make("3", 0);

  // Let us insert the updates into the appropriate timesteps.
  
  {
    cxsom::symbol::Instance res {"2", "B", 0};
    cxsom::symbol::Instance op1 {"1", "B", 0};
    auto uref = std::shared_ptr<Dummy>(new Dummy(center,                                                  // The data center
						 {res, cxsom::type::make("Pos1D")},                       // The result
						 {cxsom::update::arg(op1, cxsom::type::make("Pos1D"))})); // The lis of arguments
    uref->set_info(res, DEADLINE);
    *(timesteps[res]) += {uref}; // {init_ref, normal_ref} or {normal_ref}
  }

  {
    cxsom::symbol::Instance res {"1", "B", 0};
    cxsom::symbol::Instance op1 {"1", "A", 0};
    auto uref = std::shared_ptr<Dummy>(new Dummy(center,
						 {res, cxsom::type::make("Pos1D")},     
						 {cxsom::update::arg(op1, cxsom::type::make("Pos1D"))}));
    uref->set_info(res, DEADLINE);
    *(timesteps[res]) += {uref}; // {init_ref, normal_ref} or {normal_ref}
  }
  
  {
    cxsom::symbol::Instance res {"1", "A", 0};
    auto uref = std::shared_ptr<Dummy>(new Dummy(center,
  						 {res, cxsom::type::make("Pos1D")},     
  						 {}));
    uref->set_info(res, DEADLINE);
    *(timesteps[res]) += {uref}; // {init_ref, normal_ref} or {normal_ref}
  }

  {
    cxsom::symbol::Instance res  {"3", "A", 0};
    cxsom::symbol::Instance op11 {"2", "B", 0};
    auto uref_init = std::shared_ptr<Dummy>(new Dummy(center,
						      {res, cxsom::type::make("Pos1D")},     
						      {cxsom::update::arg(op11, cxsom::type::make("Pos1D"))}));
    cxsom::symbol::Instance op12 {"3", "B", 0};
    auto uref = std::shared_ptr<Dummy>(new Dummy(center,
						 {res, cxsom::type::make("Pos1D")},     
						 {cxsom::update::arg(op12, cxsom::type::make("Pos1D"))}));
    uref->set_info(res, DEADLINE);
    *(timesteps[res]) += {uref_init, uref}; // {init_ref, normal_ref} or {normal_ref}
  }

  {
    cxsom::symbol::Instance res {"3", "B", 0};
    cxsom::symbol::Instance op1 {"3", "A", 0};
    auto uref = std::shared_ptr<Dummy>(new Dummy(center,
						 {res, cxsom::type::make("Pos1D")},     
						 {cxsom::update::arg(op1, cxsom::type::make("Pos1D"))}));
    uref->set_info(res, DEADLINE);
    *(timesteps[res]) += {uref}; // {init_ref, normal_ref} or {normal_ref}
  }
  
  flush_tasks(timesteps, tasks, blockees);
  
  return 0;
}


inline void notify_blocked(blockees_map& blockees, cxsom::timestep::ref me) {
  auto [begin, end] = me->new_blockers_range();
  for(auto it = begin; it != end; ++it) blockees[*it].insert(me);
  me->acq_new_blockers();
}

// This collects all terminated timesteps consecutive to the termination of me.
template<typename OutputIt>
void notify_done(blockees_map& blockees, const cxsom::symbol::TimeStep& me, OutputIt& done_it) {
  *(done_it++) = me;
  if(auto it = blockees.find(me); it != blockees.end()) {
    for(auto ts : it->second)
      ts->notify_unblock(me, [&blockees, &done_it](const auto& me){notify_done(blockees, me, done_it);});
  }
}

template<typename TP, typename T, typename B>
void flush_tasks(TP& timesteps, T& tasks, B& blockees) {
  std::vector<cxsom::symbol::TimeStep> terminated_ts;
  unsigned int step = 0;
  cxsom::logger->msg("#################");
  cxsom::logger->msg("#################");
  cxsom::logger->push();
  while(true) {
    cxsom::logger->msg(std::string("\n################# ") + std::to_string(step++));
    {
      std::ostringstream ostr;
      ostr << std::endl
	   << "# TimeSteps";
      cxsom::logger->msg(ostr.str());
    }
    cxsom::logger->push();
    for(auto& kv : timesteps) {
      std::ostringstream ostr;
      ostr << *(kv.second);
      cxsom::logger->msg(ostr.str());
    }
    cxsom::logger->pop();
    if(tasks.empty()) {
      cxsom::logger->msg("No tasks, getting jobs...");
      for(auto& kv : timesteps) kv.second->get_jobs(kv.second, std::back_inserter(tasks));
    }
    if(tasks.empty()) {
      cxsom::logger->msg("No more work to do. Aborting.");
      return;
    }
    {
      std::ostringstream ostr;
      ostr << std::endl
	   << "# Tasks";
      cxsom::logger->msg(ostr.str());
    }
    cxsom::logger->push();
    for(auto& task : tasks) {
      std::ostringstream ostr;
      ostr << task;
      cxsom::logger->msg(ostr.str());
    }
    cxsom::logger->pop();

    auto task = tasks.front();
    tasks.pop_front();
    {
      std::ostringstream ostr;
      ostr << std::endl << "# Execution";
      cxsom::logger->msg(ostr.str());
    }

    {
      std::ostringstream ostr;
      ostr << task;
      cxsom::logger->_msg(ostr.str());
    }
		   

    auto done_it = std::back_inserter(terminated_ts);
    task([&blockees](auto me){notify_blocked(blockees, me);},
	 [&blockees, &done_it](auto me){notify_done(blockees, me, done_it);});
    for(auto& ts : terminated_ts)
      if(auto it = blockees.find(ts); it != blockees.end() /* should be true */)
	blockees.erase(it);
    terminated_ts.clear();
    
  }
  cxsom::logger->pop();
}
