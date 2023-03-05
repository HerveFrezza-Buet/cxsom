#pragma once

#include <iostream>
#include <string>
#include <iterator>
#include <map>
#include <tuple>

#include <cxsomProtocol.hpp>
#include <cxsomData.hpp>

namespace cxsom {
  namespace protocol {
    namespace read {

      inline symbol::Variable variable(std::istream& is) {
	symbol::Variable res;
	char c;
	is >> res.timeline;
	is >> c;
	if(c != '/') is.putback(c);
	is >> res.name;
	is.get(c);
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << "-- variable(...) : got " << res << std::endl;
#endif
	return res;
      }
      
      inline type::ref type_value(std::istream& is) {
	std::string buf;
	char c;
	is >> buf;
	is.get(c);
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << "-- type_value(\"" << buf << "\")." << std::endl;
#endif
	return type::make(buf);
      }

      inline std::tuple<std::size_t, std::size_t, bool> storage(std::istream& is) {
	std::size_t cache_size;
	std::size_t file_size;
	bool        kept_opened;
	char        c;
	is >> cache_size >> file_size >> kept_opened;
	is.get(c);
	return {cache_size, file_size, kept_opened};
      }


      
      template<typename Fct>
      void type_declaration(std::istream& is, const Fct& fct) {
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << "-- type_declaration(...) >>>>" << std::endl;
#endif
	auto v = variable(is);
	auto t = type_value(is);
	auto [cache_size, file_size, kept_opened] = storage(is);
	fct(v, t, cache_size, file_size, kept_opened);
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << "-- type_declaration <<<<" << std::endl;
#endif
      }

      std::map<std::string, std::string> params(std::istream& is) {
	std::map<std::string, std::string> res;
	std::string buf, k, v;
	std::size_t nb;
	char sep;
	is >> buf;
	if(buf != "params") throw error::parse("keyword \"params\" expected in protocol.");
	is >> nb;
	is.get(sep);
	for(std::size_t i = 0; i < nb; ++i) {
	  is >> k >> v;
	  is.get(sep);
	  res[k] = v;
	}
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << "-- params(...) : got" << std::endl;
	for(auto& kv :res)
	  std::cout << "    \"" << kv.first << "\": \"" << kv.second << "\"" << std::endl;
#endif
	
	return res;
      }
      
      inline symbol::Instance instance(std::istream& is) {
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << "-- instance(...) >>>>" << std::endl;
#endif
	symbol::Instance res;
	char c;
	res.variable = variable(is);
	is >> c;
	if(c == '@') {
	  std::ostringstream ostr;
	  ostr << "cxsom::protocol::read::instance: Time instance starting with '@' found while parsing " << res.variable << '.';
	  throw error::parse(ostr.str());
	}
	is.putback(c);
	is >> res.at;
	is.get(c);
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << "-- instance <<<< : got " << res << std::endl;
#endif
	return res;
      }

      jobs::Function function(std::istream& is) {
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << "-- function(...) >>>>" << std::endl;
#endif
	jobs::Function res;
	std::string buf;
	std::size_t nb_args;
	char c;

	is >> buf;
	res.op = buf;
	is >> nb_args;
	is.get(c);
	auto out = std::back_inserter(res.args);
	for(std::size_t i = 0; i < nb_args; ++i) *(out++) = instance(is);

	res.params = params(is);
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << "-- function <<<< :  got " << res << std::endl;
#endif
	
	return res;
      }
      
      jobs::Update update(std::istream& is) {
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << "-- update(...) >>>>" << std::endl;
#endif
	symbol::Instance res;
	std::optional<jobs::Function> init;
	jobs::Function usual;
	std::string  tag;
	char sep;

	res = instance(is);
	is >> tag; is.get(sep);
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << "-- first tag = \"" << tag << "\", should be \"init\" or \"usual\"." << std::endl;
#endif
	if(tag == "init") {
	  init = function(is);
	  is >> tag; is.get(sep);
#ifdef cxsomDEBUG_PROTOCOL
	  std::cout << "-- second tag = \"" << tag << "\", should be \"usual\"." << std::endl;
#endif
	}

	if(tag != "usual") {
	  std::ostringstream ostr;
	  ostr << "cxsom::protocol::read::update: Expecting \"init\" or \"usual\", but tag \"" << tag << "\" found.";
	  throw error::parse(ostr.str());
	}
	usual = function(is);
	
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << "-- update <<<<" << std::endl;
#endif

	return {res, init, usual};
      }
      

      namespace pattern {
	inline symbol::pattern::ArgInstance instance(std::istream& is) {
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << "-- pattern::instance(...) >>>>" << std::endl;
#endif
	  symbol::pattern::ArgInstance res;
	  char c;
	  res.variable = variable(is);
	  is >> c;
	  if(c == '@') {
	    int val;
	    is >> val;
	    symbol::pattern::relative rel {val};
	    res.t = rel;
	  }
	  else if(c == 'x') {
	    unsigned int val;
	    is >> val;
	    symbol::pattern::scaled scl {val};
	    res.t = scl;
	  }
	  else {
	    is.putback(c);
	    unsigned int val;
	    is >> val;
	    symbol::pattern::absolute abs {val};
	    res.t = abs;
	  }
	  is.get(c);
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << "-- pattern::instance <<<< : got " << res << std::endl;
#endif
	  return res;
	}

	jobs::pattern::Function function(std::istream& is) {
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << "-- pattern::function(...) >>>>" << std::endl;
#endif
	  jobs::pattern::Function res;
	  std::string buf;
	  std::size_t nb_args;
	  char c;

	  is >> buf;
	  res.op = buf;
	  is >> nb_args;
	  is.get(c);
	  auto out = std::back_inserter(res.args);
	  for(std::size_t i = 0; i < nb_args; ++i) *(out++) = instance(is);

	  res.params = params(is);
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << "-- pattern::function <<<< :  got " << res << std::endl;
#endif
	
	  return res;
	}
      
	jobs::pattern::Update update(std::istream& is) {
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << "-- pattern::update(...) >>>>" << std::endl;
#endif
	  symbol::Variable res;
	  std::optional<jobs::pattern::Function> init {};
	  jobs::pattern::Function usual;
	  std::string  tag;
	  char sep;

	  res = variable(is);
	  is >> tag; is.get(sep);
#ifdef cxsomDEBUG_PROTOCOL
	  std::cout << "-- first tag = \"" << tag << "\", should be \"init\" or \"usual\"." << std::endl;
#endif
	  if(tag == "init") {
	    init = function(is);
	    is >> tag; is.get(sep);
#ifdef cxsomDEBUG_PROTOCOL
	    std::cout << "-- second tag = \"" << tag << "\", should be \"usual\"." << std::endl;
#endif
	  }

	  if(tag != "usual") {
	    std::ostringstream ostr;
	    ostr << "cxsom::protocol::read::update: Expecting \"init\" or \"usual\", but tag \"" << tag << "\" found.";
	    throw error::parse(ostr.str());
	  }
	  usual = function(is);
	  
#ifdef cxsomDEBUG_PROTOCOL
	std::cout << "-- pattern::update <<<<" << std::endl;
#endif

	  return {res, init, usual};
	}

	
      }
    }
  }
}
