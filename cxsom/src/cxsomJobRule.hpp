#pragma once

#include <iostream>
#include <sstream>
#include <limits>

#include <cxsomSymbols.hpp>

namespace cxsom {
  
  namespace error {
    struct bad_operation : public std::logic_error {
      using std::logic_error::logic_error;
    };
    
    struct unhandled_type : public std::logic_error {
      using std::logic_error::logic_error;
    };

    struct bad_typing : public std::logic_error {
      using std::logic_error::logic_error;
    };
  }
  
  namespace jobs {
    
    enum class Operation : unsigned int {
      Copy,
      Average,
      Random,
      Converge,
      Clear,
      Merge,
      MatchTriangle,
      MatchGaussian,
      LearnTriangle,
      LearnGaussian,
      Argmax,
      ConvArgmax,
      TowardConvArgmax
    };

    inline std::ostream& operator<<(std::ostream& os, Operation op) {
      switch(op) {
      case Operation::Copy             : os << "copy"              ; break;
      case Operation::Average          : os << "average"           ; break;
      case Operation::Random           : os << "random"            ; break;
      case Operation::Converge         : os << "converge"          ; break;
      case Operation::Clear            : os << "clear"             ; break;
      case Operation::Merge            : os << "merge"             ; break;
      case Operation::MatchTriangle    : os << "match-triangle"    ; break;
      case Operation::MatchGaussian    : os << "match-gaussian"    ; break;
      case Operation::LearnTriangle    : os << "learn-triangle"    ; break;
      case Operation::LearnGaussian    : os << "learn-gaussian"    ; break;
      case Operation::Argmax           : os << "argmax"            ; break;
      case Operation::ConvArgmax       : os << "conv-argmax"       ; break;
      case Operation::TowardConvArgmax : os << "toward-conv-argmax"; break;
      }
      return os;
    }

    Operation operation(const std::string& s) {
      if(s == "copy"              ) return Operation::Copy;
      if(s == "average"           ) return Operation::Average;
      if(s == "random"            ) return Operation::Random;
      if(s == "converge"          ) return Operation::Converge;
      if(s == "clear"             ) return Operation::Clear;
      if(s == "merge"             ) return Operation::Merge;
      if(s == "match-triangle"    ) return Operation::MatchTriangle;
      if(s == "match-gaussian"    ) return Operation::MatchGaussian;
      if(s == "learn-triangle"    ) return Operation::LearnTriangle;
      if(s == "learn-gaussian"    ) return Operation::LearnGaussian;
      if(s == "argmax"            ) return Operation::Argmax;
      if(s == "conv-argmax"       ) return Operation::ConvArgmax;
      if(s == "toward-conv-argmax") return Operation::TowardConvArgmax;
      throw error::bad_operation(std::string("cxsom::jobs::operation(\"") + s + "\") : unhandled operation.");
    }

    struct Function {
      Operation op;
      std::vector<symbol::Instance> args;
      std::map<std::string, std::string> params;
    };


    std::ostream& operator<<(std::ostream& os, const Function& f) {
      os << f.op << '(';
      auto it = f.args.begin();
      if(it != f.args.end()) os << *(it++);
      while(it != f.args.end()) os << ", " << *(it++);
      os << ')';
      return os;
    }
    
    struct Update {
      symbol::Instance res;
      std::optional<Function> init;
      Function                usual;
    };

    std::ostream& operator<<(std::ostream& os, const Update& u) {
      std::ostringstream ostr; 
      ostr << u.res;
      if(u.init) os << ostr.str() << " <= " << *(u.init) << std::endl
		    << std::string(ostr.str().size(), ' ') << " << " << u.usual;
      else os << ostr.str() << " << " << u.usual;
      return os;
    }
    
    Update make(const symbol::Instance& res, const Function& init, const Function& usual) {
      return {res, init, usual};
    }

    Update make(const symbol::Instance& res, const Function& usual) {
      return {res, std::nullopt, usual};
    }
    
    namespace pattern {
      struct Function {
	int min_offset;
	Operation op;
	std::vector<symbol::pattern::ArgInstance> args;
	std::map<std::string, std::string> params;
	Function(Operation op,
		 const std::vector<symbol::pattern::ArgInstance>& args,
		 const std::map<std::string, std::string>& params)
	  : min_offset(0), op(op), args(args), params(params) {
	  for(auto& a: args)
	    if(std::holds_alternative<int>(a.t))
	      if(int offset = std::get<int>(a.t); offset < min_offset)
		min_offset = offset;
	}
	Function()                           = default;
	Function(const Function&)            = default;
	Function& operator=(const Function&) = default;
      };

      std::ostream& operator<<(std::ostream& os, const Function& f) {
	os << f.op << '(';
	auto it = f.args.begin();
	if(it != f.args.end()) os << *(it++);
	while(it != f.args.end()) os << ", " << *(it++);
	os << ')';
	return os;
      }

      struct Update {
	int min_offset;
	symbol::Variable             res;
	std::optional<Function>      init;
	Function                     usual;
	unsigned int                 walltime;

	Update() = delete;
	Update(const symbol::Variable& res, const std::optional<Function>& init, const Function& usual, unsigned int walltime)
	  : min_offset(0), res(res), init(init), usual(usual), walltime(walltime) {
	  if(init) min_offset = std::min(init->min_offset, usual.min_offset);
	  else     min_offset = usual.min_offset;
	}
	
	Update(const symbol::Variable& res, const std::optional<Function>& init, const Function& usual)
	  : Update(res, init, usual, 0) {
	  if(auto it = usual.params.find("walltime"); it != usual.params.end()) {
	    if(std::stod(it->second) < 0)
	      walltime = std::numeric_limits<unsigned int>::max();
	    else
	      walltime = std::stoul(it->second);
	  }
	}
	
	Update(const Update&)            = default;
	Update& operator=(const Update&) = default;
      };
      
      std::ostream& operator<<(std::ostream& os, const Update& u) {
	std::ostringstream ostr; 
	ostr << u.res;
	if(u.init) os << ostr.str() << " <= " << *(u.init) << std::endl
		      << std::string(ostr.str().size(), ' ') << " << " << u.usual;
	else os << ostr.str() << " << " << u.usual;
	return os;
      }


      jobs::Function at(const Function& f, unsigned int ref_time) {
	std::vector<symbol::Instance> args;
	auto out =  std::back_inserter(args);
	for(auto& arg : f.args) *(out++) = arg.at(ref_time);
	return {f.op, std::move(args), f.params};
      }

      jobs::Update at(const Update& updt, unsigned int ref_time) {
#ifdef cxsomDEBUG_TASKS
	std::cout << "  realizing pattern at " << ref_time << ':' << std::endl
		  << updt << std::endl
		  << std::endl;
					      
#endif
	if(updt.init)
	  return {{updt.res, ref_time}, at(*(updt.init), ref_time), at(updt.usual, ref_time)};
	else
	  return {{updt.res, ref_time}, std::nullopt, at(updt.usual, ref_time)};
      }

      Update make(const symbol::Variable& res, const Function& init, const Function& usual, unsigned int walltime) {
	return {res, init, usual, walltime};
      }

      Update make(const symbol::Variable& res, const Function& usual, unsigned int walltime) {
	return {res, std::nullopt, usual, walltime};
      }
    }
  }
}
