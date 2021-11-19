#pragma once

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <optional>

#include <cxsomProtocol.hpp>
#include <cxsomRuleDefs.hpp>


namespace cxsom {
  namespace protocol {
    namespace write {
      inline void variable  (std::ostream& os, const rules::value_key& v) {os << v.timeline << ' ' << v.name << ' ';}
      inline void type_value(std::ostream& os, const std::string& t     ) {os << t << ' ';}
      inline void storage   (std::ostream& os, std::size_t cache_size,
			     std::size_t file_size, bool kept_opened)    {os << cache_size << ' ' << file_size << ' ' << kept_opened << ' ';}

      inline void type_declaration(std::ostream& os,
				   const rules::value_key& v,
				   const std::string& t,
				   std::size_t cache_size,
				   std::size_t file_size,
				   bool kept_opened) {
#ifdef cxsomDEBUG_PROTOCOL
	std::ostringstream ss;
	std::ostream& ostr = ss;
#else
	std::ostream& ostr = os;
#endif
	
	ostr << "declare "; variable(ostr, v); type_value(ostr, t); storage(ostr, cache_size, file_size, kept_opened); ostr << std::endl;
	
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << "<< " << ss.str(); os << ss.str();
#endif
	
      }

      inline void params(std::ostream& os, const std::vector<rules::kwd::param>& params) {
	os << "params " << params.size() << ' ';
	for(auto& p : params) os << p.name << ' ' << std::setprecision(15) << p.value << ' ';
      }
      
      inline void instance  (std::ostream& os, const rules::value_at_key& i) {
	variable(os, i.key);
	os << std::get<unsigned int>(i.date) << ' ';
      }
	  
      inline void update(std::ostream& os,
			 const rules::update& u) {
	os << u.name << ' ' << u.args.size() << ' ';
	for(auto& arg : u.args) instance(os, arg.id);
	params(os, u.params);
      }
      
      inline void update(std::ostream& os, const rules::value_at_key& res,
			 const std::optional<rules::update>& init,
			 const rules::update& usual) {          
#ifdef cxsomDEBUG_PROTOCOL
	std::ostringstream ss;
	std::ostream& ostr = ss;
#else
	std::ostream& ostr = os;
#endif
	ostr << "static ";
	instance(ostr, res);
	if(init) {
	  ostr << "init ";
	  update(ostr, *init);
	}
	ostr << "usual ";
	update(ostr, usual);
	ostr << std::endl;
	
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << "<< " << ss.str(); os << ss.str();
#endif
      }

      inline void updates(std::ostream& os, std::size_t number) {     
#ifdef cxsomDEBUG_PROTOCOL
	std::ostringstream ss;
	std::ostream& ostr = ss;
#else
	std::ostream& ostr = os;
#endif
	ostr << "updates " <<  number << ' ' << std::endl;
	
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << "<< " << ss.str(); os << ss.str();
#endif
      }


      namespace pattern {
	inline void instance  (std::ostream& os, const rules::value_at_key& i) {
	  variable(os, i.key);
	  os << i.date << ' ';
	}
	  
	inline void update(std::ostream& os,
			   const rules::update& u) {
	  os << u.name << ' ' << u.args.size() << ' ';
	  for(auto& arg : u.args) instance(os, arg.id);
	  params(os, u.params);
	}
      
	inline void update(std::ostream& os, const rules::value_at_key& res,
			   const std::optional<rules::update>& init,
			   const rules::update& usual) {       
#ifdef cxsomDEBUG_PROTOCOL
	  std::ostringstream ss;
	  std::ostream& ostr = ss;
#else
	  std::ostream& ostr = os;
#endif
	  ostr << "pattern ";
	  variable(ostr, res.key);
	  if(init) {
	    ostr << "init ";
	    update(ostr, *init);
	  }
	  ostr << "usual ";
	  update(ostr, usual);
	  ostr << std::endl;
	
#ifdef cxsomDEBUG_PROTOCOL
	  std::cout << "<< " << ss.str(); os << ss.str();
#endif
	}
      }
      
    }
  }
}
