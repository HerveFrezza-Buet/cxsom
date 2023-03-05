#pragma once

#include <cstddef>
#include <string>
#include <memory>

#include <cxsom-rules.hpp>
#include <cxsombuilderDot.hpp>

using namespace cxsom::rules;

namespace cxsom {
  namespace builder {
    
    struct name {
      std::string value;
      name operator/(const name& n) {return value + "/" + n.value;}
      name operator/(const std::string& n) {return (*this) / name(n);}
      name()                       = default;
      name(const name&)            = default;
      name(name&&)                 = default;
      name& operator=(const name&) = default;
      name& operator=(name&&)      = default;
      name(const std::string& value) : value(value) {}
      operator std::string () const {return value;}
    };
    
    struct Variable : public Dot {
      std::string timeline;
      name        varname;
      std::string type;
      std::size_t cache_size;
      std::size_t file_size;
      bool        kept_opened;
      Variable(const std::string& timeline,
	       const name&        varname,
	       const std::string& type,
	       std::size_t        cache_size,
	       std::size_t        file_size,
	       bool               kept_opened)
	: Dot("v", "box", "filled", "#aa88ff", varname.value, timeline),
	  timeline(timeline), varname(varname), type(type), cache_size(cache_size), file_size(file_size), kept_opened(kept_opened) {}
      
      void definition() const {
	kwd::type(var(), type, cache_size, file_size, kept_opened);
      }

      kwd::data var() const {return kwd::var(timeline, varname);}
    };

    std::ostream& operator<<(std::ostream& os, const Variable& v) {
      return os << '[' << v.timeline << ':' << std::string(v.varname) << ", " << v.type << ']';
    }
    

    inline auto variable(const std::string& timeline,
			 const name&        varname,
			 const std::string& type,
			 std::size_t        cache_size,
			 std::size_t        file_size,
			 bool               kept_opened) {
      return std::make_shared<Variable>(timeline, varname, type, cache_size, file_size, kept_opened);
    }

    struct AnalysisContext {
      std::string  external_prefix;
      std::string  internal_prefix;
      std::size_t  cache_size;
      std::size_t  file_size;
      bool         kept_opened;
      unsigned int at;
      bool         full_record;
      mutable bool external_prefix_mode = false;

      const std::string& prefix() const {
	if(external_prefix_mode) return external_prefix;
	return internal_prefix;
      }
      
      AnalysisContext(const std::string& external_prefix, const std::string& internal_prefix,
		      std::size_t cache_size, std::size_t file_size,
		      bool kept_opened, unsigned int at,
		      bool full_record)
	: external_prefix(external_prefix), internal_prefix(internal_prefix), cache_size(cache_size), file_size(file_size), kept_opened(kept_opened), at(at), full_record(full_record) {}
      
      std::shared_ptr<Variable> operator()(std::shared_ptr<Variable> var) const {
	return variable(prefix() + "-" + var->timeline,
			var->varname, var->type, cache_size, file_size, kept_opened);
      }
      
      std::shared_ptr<Variable> operator[](std::shared_ptr<Variable> var) const {
	std::size_t f_size = 0;
	if(full_record) f_size = file_size;
	return variable(prefix() + "-" + var->timeline,
			var->varname, var->type, cache_size, f_size, kept_opened);
      }

      void operator=(LayerKind k) const {
	switch(k) {
	case LayerKind::None:
	case LayerKind::ContextualAdaptive:
	case LayerKind::ContextualStatic:
	  external_prefix_mode = false;
	  break;
	case LayerKind::ExternalAdaptive:
	case LayerKind::ExternalStatic:
	  external_prefix_mode = true;
	  break;
	default:
	  external_prefix_mode = false;
	  break;
	}
      }
    };



    namespace timestep {
      auto absolute(unsigned int at) {return at;}
      auto relative(int shift)       {return rules::offset(shift);}
      auto previous()                {return rules::offset(-1);}
      auto current ()                {return rules::offset(0);}
    }
  }
}
