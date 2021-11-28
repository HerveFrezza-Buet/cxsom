#pragma once

#include <vector>
#include <algorithm>
#include <iterator>
#include <initializer_list>
#include <cxsomSymbols.hpp>
#include <cxsomVariable.hpp>
#include <map>
#include <iostream>
#include <tuple>
#include <optional>
#include <sstream>

#include <cxsomLog.hpp>

namespace cxsom {
  namespace update {

    enum class Status : char {
      Impossible = 'i', //!< Update is Busy  : Computation is not feasible yet, since some out-args are busy.
      UpToDate   = 'u', //!< Update is Busy  : Nothing changed in the in-args input dates from last update, or the new value was not an actual modification.
      Updated    = 'U', //!< Update is Busy  : The computation has modified the value.
      Done       = 'd', //!< Update is Ready : The computation has modified the value definitively.
      None       = 's'  //!< Not determined.
    };


    inline std::ostream& operator<<(std::ostream& os, const Status& s) {
      switch(s) {
      case Status::Impossible : os << "impossible"; break;
      case Status::UpToDate   : os << "up-to-date"; break;
      case Status::Updated    : os << "updated"   ; break;
      case Status::Done       : os << "done"      ; break;
      case Status::None       : os << "none"      ; break;
      };
      return os;
    }
    
    using arg = std::tuple<symbol::Instance, type::ref>;
    
    /**
     * This is a rule for setting one instance value from other instances.
     */
    class Base {
    public:

      struct Instance {
	symbol::Instance   who;
	data::instance_ref what = nullptr;
	Instance()                           = default;
	Instance(const Instance&)            = default;
	Instance& operator=(const Instance&) = default;
	Instance(const symbol::Instance& who, data::instance_ref what) : who(who), what(what) {}
	bool operator<(const Instance& other) const {return who < other.who;}
      };
      
      struct ArgInstance : public Instance {
	cxsom::data::Availability availability = cxsom::data::Availability::Busy;
	unsigned int arg_number = 0;
	ArgInstance()                              = default;
	ArgInstance(const ArgInstance&)            = default;
	ArgInstance& operator=(const ArgInstance&) = default;
	ArgInstance(const symbol::Instance& who, data::instance_ref what, unsigned int arg_number) : Instance(who, what), arg_number(arg_number) {}
      };

      struct InArgInstance : public ArgInstance {
      private:
	friend class Base;
	std::optional<unsigned int> update = std::nullopt;
	unsigned int date = 0; // 0 means no-update.

	friend class ResInstance;

      public:
	
	InArgInstance()                                = default;
	InArgInstance(const InArgInstance&)            = default;
	InArgInstance& operator=(const InArgInstance&) = default;
	InArgInstance(const symbol::Instance& who, data::instance_ref what, unsigned int arg_number)
	  : ArgInstance(who, what, arg_number),
	    update(), date(0) {}
	
	void notify_datation(cxsom::data::Availability a, unsigned int datation) {
	  bool get_ready = (a == cxsom::data::Availability::Ready) && (availability ==  cxsom::data::Availability::Busy);
	  availability = a;
#ifdef cxsomLOG
	  {
	    std::ostringstream ostr;
	    ostr << "in-arg " << who << " dates : " << datation << '(' << date << "), get_ready = "
		      << std::boolalpha << get_ready;
	    logger->msg(ostr.str());
	  }
#endif
	  if(get_ready || datation > date) {
	    date = datation;
	    update = date;
#ifdef cxsomLOG
	    logger->push();
	    logger->msg(std::string("updated to ") + std::to_string(date));
	    logger->pop();
#endif
	  }
	  else {
	    update = std::nullopt;
#ifdef cxsomLOG
	    logger->push();
	    logger->msg("not updated");
	    logger->pop();
#endif
	  }
	}
      };

      struct OutArgInstance : public ArgInstance {
      private:
	friend class Base;
      public:
	
	OutArgInstance()                                 = default;
	OutArgInstance(const OutArgInstance&)            = default;
	OutArgInstance& operator=(const OutArgInstance&) = default;
	OutArgInstance(const symbol::Instance& who, data::instance_ref what, unsigned int arg_number)
	  : ArgInstance(who, what, arg_number) {} 
	bool notify_status(cxsom::data::Availability a) {
	  availability = a;
	  return a == cxsom::data::Availability::Ready;}
	operator bool() const {return availability != cxsom::data::Availability::Ready;}
      };
      
      struct ResInstance : public Instance {
      private:
	std::optional<unsigned int> max_in_args_date = 0;
	bool all_ready = false;
	
      public:
	bool updated_once = false;
	Status status;
	ResInstance()                              = default;
	ResInstance(const ResInstance&)            = default;
	ResInstance& operator=(const ResInstance&) = default;
	ResInstance(const symbol::Instance& who, data::instance_ref what)
	  : Instance(who, what), status(Status::Updated) {}
	
	/**
	 * This updates the date from in
	 */
	void operator<=(const InArgInstance& in_arg) {
	  if(in_arg.update) {
	    if(max_in_args_date) max_in_args_date = std::max(*max_in_args_date, *(in_arg.update));
	    else                 max_in_args_date = *(in_arg.update);
#ifdef cxsomLOG
	    {
	      std::ostringstream ostr;
	      ostr << "res " << who << " changes max in_arg date to " << *max_in_args_date << '.';
	      logger->msg(ostr.str());
	    }
#endif
	  }
	  all_ready = all_ready && (in_arg.availability == cxsom::data::Availability::Ready);
	}

	bool all_args_in_ready() {return all_ready;}

	void init_in_arg_date() {
	  all_ready = true;
	  max_in_args_date = std::nullopt;
	}
	
	bool no_in_arg_update() {return !max_in_args_date;}

	unsigned int set_date(unsigned int current_date) {
#ifdef cxsomLOG
	  {
	    std::ostringstream ostr;
	    ostr << "res " << who << " computes its new date (current date is " << current_date << ").";
	    logger->msg(ostr.str());
	  }
#endif	  
	  if(max_in_args_date) {
#ifdef cxsomLOG
	    logger->push();
	    logger->msg(std::string("A max date has been computed as ")
		     + std::to_string(*max_in_args_date) + ", so the result date is "
		     + std::to_string(1+std::max(current_date, *max_in_args_date)) + ".");
	    logger->pop();
#endif	  
	    return 1+std::max(current_date, *max_in_args_date);
	  }
	  else {
#ifdef cxsomLOG
	    logger->push();
	    logger->msg(std::string("No max date have been computed, so the result date is ")
		     + std::to_string(1+current_date) + ".");
	    logger->pop();
#endif	  
	    return 1+current_date;
	  }
	}
      };

    public:
      
      ResInstance                 result;
      std::vector<InArgInstance>  args_in;
      std::vector<OutArgInstance> args_out;

    private:

      mutable std::vector<symbol::TimeStep> blockers;
      friend std::ostream& operator<<(std::ostream& os, const Base& base);

    public:

      bool is_init = false;
      bool out_ok  = false;
      
      Base()                       = delete;
      Base(const Base&)            = default;
      Base& operator=(const Base&) = default;
      Base(Base&&)                 = default;
      Base& operator=(Base&&)      = default;

      operator symbol::Instance() const {return result.who;}

      Base(data::Center& center,
	   const arg& res,
	   const std::vector<arg>& args) {

	
	// Let us check all the compoments.
	center.check(std::get<0>(res), std::get<1>(res));

	
	for(auto& arg : args) center.check(std::get<0>(arg), std::get<1>(arg));

	
	// Let us now build the instances.
	result = {std::get<0>(res), center[std::get<0>(res)]};
	auto res_time_step = (symbol::TimeStep)(std::get<0>(res));
	auto out_in  = std::back_inserter(args_in );
	auto out_out = std::back_inserter(args_out);
	unsigned int arg_id = 0;
	for(auto& arg : args)
	  if((symbol::TimeStep)(std::get<0>(arg)) == res_time_step)
	    *(out_in++)  = {std::get<0>(arg), center[std::get<0>(arg)], arg_id++};
	  else
	    *(out_out++) = {std::get<0>(arg), center[std::get<0>(arg)], arg_id++};
	
      }

      using blockers_iter_type = std::vector<symbol::TimeStep>::iterator;
      std::pair<blockers_iter_type, blockers_iter_type> blockers_range() const {
	blockers.clear();
	auto out = std::back_inserter(blockers);
	for(auto& arg : args_out) if(arg) *(out++) = arg.who;
	return {blockers.begin(), blockers.end()};
      }

      void set_ready() {
	// This is never called for init updates.
	result.what->set([](auto& status, auto&, auto&) {status = data::Availability::Ready;});
#ifdef cxsomLOG
	std::ostringstream ostr;
	ostr << "Setting update of " << result.who << " as ready.";
	logger->msg(ostr.str());
#endif
      }


      
      Status operator()() {
#ifdef cxsomLOG
	{
	  std::ostringstream ostr;
	  ostr << std::endl << "Updating " << result.who << '.';
	  logger->msg(ostr.str());
	  logger->push();
	}
#endif
	data::Availability res_status;
	result.what->set([&res_status](auto& status, auto&, auto&) {res_status = status;});
	if(res_status == data::Availability::Ready) {
#ifdef cxsomLOG
	  logger->msg("result is already ready. returning Done");
	  logger->pop();
#endif
	  return cxsom::update::Status::Done;
	}
	
	on_computation_start();
#ifdef cxsomLOG
	logger->msg("inspecting out...");
#endif
	if(!out_ok) {
	  out_ok = true;
	  for(auto& arg : args_out) 
	    arg.what->get([&arg, this](auto status, auto, auto& data) {
		out_ok = arg.notify_status(status) && out_ok;
		if(out_ok) on_read_out_arg(arg.who, arg.arg_number, data);
	      });
	  if(!out_ok) {
	    on_read_out_arg_aborted();
#ifdef cxsomLOG
	    logger->msg("out inspection aborted, returning Impossible");
	    logger->pop();
#endif
	    return cxsom::update::Status::Impossible;
	  }
	}

#ifdef cxsomLOG
	logger->msg("inspecting in...");
#endif
	result.init_in_arg_date();
	if(args_in.size() != 0) {
	  for(auto& arg : args_in) 
	    arg.what->get([this, &arg](auto status, auto datation, auto& data) {
		arg.notify_datation(status, datation);
		on_read_in_arg(arg.who, arg.arg_number, data);
		result <= arg;
	      });
	  if(result.no_in_arg_update() && result.updated_once) { // No need for recomputing the result.
	    if(is_init) {
#ifdef cxsomLOG
	      logger->msg("no in-arg update and not the first update, but it was an init. Return Updated.");
	      logger->pop();
#endif
	      return cxsom::update::Status::Updated;
	    }
	    else {
#ifdef cxsomLOG
	      logger->msg("no in-arg update and not the first update, so we are up to date. Retrun UpToDate.");
	      logger->pop();
#endif
	      return cxsom::update::Status::UpToDate;
	    }
	  }
	}
	
	cxsom::update::Status exec_status;
	result.what->set([this, &exec_status](auto& status, auto& datation, auto& data) {
#ifdef cxsomLOG
	    logger->msg("we compute the result...");
#endif
	    bool significant_write = on_write_result(data);
	    if(significant_write || !result.updated_once){ // The data actually changed
#ifdef cxsomLOG
	      std::ostringstream ostr;
	      ostr << "We consider a significant result computation (significant write = "
		   << std::boolalpha << significant_write << ", updated_once = " << result.updated_once
		   << "). Planning to return Updated.";
	      logger->msg(ostr.str());
#endif
	      result.updated_once = true;
	      datation = result.set_date(datation);
	      exec_status = cxsom::update::Status::Updated;
	    }
	    else if(is_init) {
	      exec_status = cxsom::update::Status::Updated;
#ifdef cxsomLOG
	      logger->msg("No result change, but it was an init. Planning to return Updated.");
#endif
	    }
	    else {
#ifdef cxsomLOG
	      logger->msg("No result change (and it is not an init). Planning to return UpToDate.");
#endif
	      exec_status = cxsom::update::Status::UpToDate;
	    }

	    if(result.all_args_in_ready()) {
	      if(is_init) {
#ifdef cxsomLOG
	      logger->msg("All arguments are ready, but this is an init. Planning to return Updated.");
#endif
		exec_status = cxsom::update::Status::Updated;
	      }
	      else {
#ifdef cxsomLOG
	      logger->msg("All arguments are ready (and it is not an init). Planning to return Done.");
#endif
		status      = cxsom::data::Availability::Ready;
		exec_status = cxsom::update::Status::Done;
	      }
	    }

#ifdef cxsomLOG
	    {
	      std::ostringstream ostr;
	      ostr << "Finally, returning " << exec_status << ", the result date is " << datation << '.';
	      logger->msg(ostr.str());
	      logger->pop();
	    }
#endif
	  });
	return exec_status;
      }
      
    public:
      virtual std::string function_name() const {return "<undocumented>";}

      /**
       * This reconsiders the availability of the arguments from the variable files.
       */
      void sync_arguments_availability() {
#ifdef cxsomLOG
	{
	  std::ostringstream ostr;
	  ostr << "Syncing update " << *this << '.';
	  logger->msg(ostr.str());
	}
	logger->push();
#endif
	for(auto& arg : args_out)
	  if(arg.availability == cxsom::data::Availability::Busy) {
	    arg.what->sync([&arg](auto status) {arg.notify_status(status);});
#ifdef cxsomLOG
	    std::ostringstream ostr;
	    ostr << "Arg " << arg.who << " was known as busy, we check it... it is " << arg.availability << ".";
	    logger->msg(ostr.str());
#endif
	  }
	  else {
#ifdef cxsomLOG
	    std::ostringstream ostr;
	    ostr << "Arg " << arg.who << " was already known as ready, no file checking.";
	    logger->msg(ostr.str());
#endif
	  }
#ifdef cxsomLOG
	logger->pop();
#endif
      }
      
    protected:

      /**
       * This is calld at computation start
       */
      virtual void on_computation_start() {}
      
      /**
       * This is called when a "out" argument is considered.
       */ 
      virtual void on_read_out_arg(const symbol::Instance&, unsigned int, const data::Base&) {}

      /**
       * This called if not all the out arguments have been considered
       * (i.e. read_out_arg has not been called for all out
       * arguments). If no abortion occurred, all out argument have
       * been read once. They will not be read anymore in next
       * computations.
       */
      virtual void on_read_out_arg_aborted() {}

      /**
       * This is called when a "in" argument is considered.
       */ 
      virtual void on_read_in_arg(const symbol::Instance&, unsigned int, const data::Base&) {}
      
      /**
       * This is called when the result is to be written. 
       * @returns true if the result value actually changed.
       */ 
      virtual bool on_write_result(data::Base&) = 0;
    };

    using  ref = std::shared_ptr<Base>;
    using wref = std::weak_ptr<Base>;

    inline std::ostream& operator<<(std::ostream& os, const Base::ResInstance& i)  {
      os << "res{" << i.who << " | " << i.status << '}';
      return os;
    }
    
    inline std::ostream& operator<<(std::ostream& os, const Base::InArgInstance& i)  {
      os << "in{" << i.who << " | #" << i.arg_number << ", " << i.availability << '}';
      return os;
    }
    
    inline std::ostream& operator<<(std::ostream& os, const Base::OutArgInstance& i)  {
      os << "out{" << i.who << " | #" << i.arg_number << ", " << i.availability << '}';
      return os;
    }
    
    inline std::ostream& operator<<(std::ostream& os, const Base& base) {
      os << base.result;
      if(base.is_init) os << " <= ";
      else             os << " << ";
      os << base.function_name() << '(';
      bool first = true;
      for(auto& a: base.args_in)  if(first) {first = false; os << a;} else os << ", " << a;
      for(auto& a: base.args_out) if(first) {first = false; os << a;} else os << ", " << a;
      os << ')';
      return os;
    }
  }
}


