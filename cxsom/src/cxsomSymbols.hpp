#pragma once

#include <tuple>
#include <variant>
#include <sstream>
#include <stdexcept>

namespace cxsom {
  namespace error {
    struct negative_time : public std::logic_error {
      using std::logic_error::logic_error;
    };
  }
  
  namespace symbol {

    namespace parse {
      std::size_t root_dir_length(const std::string& root_dir) {
	auto prefix_length = root_dir.size();
	if(prefix_length == 0) return 0;
	if(root_dir[prefix_length-1] != '/') return prefix_length + 1;
	return prefix_length;
      }
      
      std::pair<std::string, std::string> split_varpath(std::size_t prefix_length, const std::string& p) {
	auto name = p.substr(prefix_length, p.size() - prefix_length - 4);
	if(name == "") return {"", ""};
	
	auto it = name.begin();
	while((it != name.end()) && (*it != '/')) ++it;
	if(it == name.end()) return {"", name};
	std::string prefix, suffix;
	std::copy(name.begin(), it, std::back_inserter(prefix));
	std::copy(it+1, name.end(), std::back_inserter(suffix));
	
	return {prefix, suffix};
      }
    }
    

    /**
     * This is the name of a quantity living its life inside a timeline.
     */
    struct Variable {
      std::string timeline;
      std::string name;
      Variable(const std::string& timeline, const std::string& name) : timeline(timeline), name(name) {}
      Variable(const std::pair<std::string, std::string>& t_n) : Variable(t_n.first, t_n.second) {}
      Variable()                           = default;
      Variable(const Variable&)            = default;
      Variable& operator=(const Variable&) = default;
      Variable(Variable&&)                 = default;
      Variable& operator=(Variable&&)      = default;
     
      int  compare   (const Variable& v2) const {int cmp = timeline.compare(v2.timeline); if(cmp == 0) return name.compare(v2.name); else return cmp;}
      bool operator< (const Variable& v2) const {int cmp = timeline.compare(v2.timeline); return cmp < 0 || (cmp == 0 && name < v2.name);}
      bool operator==(const Variable& v2) const {return timeline == v2.timeline && name == v2.name;}
    };

    inline std::ostream& operator<<(std::ostream& os, const Variable& v) {
      os << '[' << v.timeline << ", " << v.name << ']';
      return os;
    }


    /**
     * This is the group of instances at the same timeline at the same time step..
     */
    struct TimeStep {
      std::string timeline;
      unsigned int at;

      TimeStep(const std::string& timeline, unsigned int at) : timeline(timeline), at(at) {}
      TimeStep()                           = default;
      TimeStep(const TimeStep&)            = default;
      TimeStep& operator=(const TimeStep&) = default;
      TimeStep(TimeStep&&)                 = default;
      TimeStep& operator=(TimeStep&&)      = default;

      operator std::string  () const {return timeline;}
      operator unsigned int () const {return at;}
      
      bool operator< (const TimeStep& v2) const {int cmp = timeline.compare(v2.timeline); return cmp < 0 || (cmp == 0 && at < v2.at);}
      bool operator==(const TimeStep& v2) const {return timeline == v2.timeline && at == v2.at;}
    };

    inline std::ostream& operator<<(std::ostream& os, const TimeStep& i) {
      os << i.timeline << '@' << i.at;
      return os;
    }

    /**
     * This is the name of a specific instance of a variable in a timeline.
     */
    struct Instance {
      Variable variable;
      unsigned int at;

      Instance(const std::string& timeline, const std::string& name, unsigned int at) : variable(timeline, name), at(at) {}
      Instance(const Variable& variable, unsigned int at) : variable(variable), at(at) {}
      Instance()                           = default;
      Instance(const Instance&)            = default;
      Instance& operator=(const Instance&) = default;
      Instance(Instance&&)                 = default;
      Instance& operator=(Instance&&)      = default;

      bool belongs(const TimeStep& ts) const {
	return (ts.at == at) && (ts.timeline == variable.timeline);
      }

      operator Variable     () const {return variable;}
      operator unsigned int () const {return at;}
      operator TimeStep     () const {return {variable.timeline, at};}
      
      bool operator< (const Instance& v2) const {int cmp = variable.compare(v2.variable); return cmp < 0 || (cmp == 0 && at < v2.at);}
      bool operator==(const Instance& v2) const {return variable == v2.variable && at == v2.at;}
    };

    inline std::ostream& operator<<(std::ostream& os, const Instance& i) {
      os << i.variable << '@' << i.at;
      return os;
    }

    namespace pattern {
      struct absolute{unsigned int at;};
      struct relative{
	int offset;
	relative operator-() const {return {-offset};}
      };
      struct scaled{unsigned int factor;};
      using time = std::variant<absolute, relative, scaled>;
    }
  }
}

inline cxsom::symbol::pattern::absolute operator""_absolute  (unsigned long long int v) {return {(unsigned int)v};}
inline cxsom::symbol::pattern::relative operator""_relative  (unsigned long long int v) {return {         (int)v};}
inline cxsom::symbol::pattern::scaled   operator""_scaled    (unsigned long long int v) {return {(unsigned int)v};}

namespace cxsom {
  namespace symbol {
    namespace pattern {
      
      /**
       * This is the name of a specific argument instance pattern of a variable in a timeline.
       */
      struct ArgInstance {
	Variable variable;
	time t;

	ArgInstance(const std::string& timeline, const std::string& name, time t) : variable(timeline, name), t(t) {}
	ArgInstance(const Variable& variable, time t) : variable(variable), t(t) {}
	ArgInstance(const std::string& timeline, const std::string& name) : ArgInstance(timeline, name, 0_absolute) {}
	ArgInstance(const Variable& variable) : ArgInstance(variable, 0_absolute) {}
	ArgInstance()                              = default;
	ArgInstance(const ArgInstance&)            = default;
	ArgInstance& operator=(const ArgInstance&) = default;
	ArgInstance(ArgInstance&&)                 = default;
	ArgInstance& operator=(ArgInstance&&)      = default;

	operator Variable     () const {return variable;}

	symbol::Instance at(unsigned int ref_time) const {
	  if(std::holds_alternative<absolute>(t))
	    return {variable, std::get<absolute>(t).at};
	  else if(std::holds_alternative<relative>(t)) {
	    int offset = std::get<relative>(t).offset;
	    if(offset < 0 && (unsigned int)(-offset) > ref_time) 
	      throw error::negative_time(std::string("negative time : ") + std::to_string(ref_time) + std::to_string(offset));
	    return {variable, (unsigned int)(ref_time + offset)};
	  }
	  else
	    return {variable, ref_time * std::get<scaled>(t).factor};
	}
      };

      inline std::ostream& operator<<(std::ostream& os, const ArgInstance& i) {
	os << i.variable;
	if(std::holds_alternative<absolute>(i.t))
	  os << '@' << std::get<absolute>(i.t).at;
	else if (std::holds_alternative<relative>(i.t))
	  os << "@+{" << std::get<relative>(i.t).offset << '}';
	else
	  os << "@*{" << std::get<scaled>(i.t).factor << '}';
	return os;
      }
      
    }
  }
  
}
