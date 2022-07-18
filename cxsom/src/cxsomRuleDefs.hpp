#pragma once




#include <stack>
#include <string>
#include <algorithm>
#include <vector>
#include <stdexcept>
#include <map>
#include <memory>
#include <tuple>
#include <variant>
#include <set>
#include <iterator>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <boost/asio.hpp>

namespace cxsom {
  namespace rules {

    namespace error {
      
      
      struct unknown_variable : public std::logic_error {
	using std::logic_error::logic_error;
      };

      struct redeclare_type : public std::logic_error {
	using std::logic_error::logic_error;
      };
      
      struct bad_update : public std::logic_error {
	using std::logic_error::logic_error;
      };
      
      struct double_update : public std::logic_error {
	using std::logic_error::logic_error;
      };
    
      struct invalid_rename : public std::logic_error {
	using std::logic_error::logic_error;
      };
    }
  
    struct context;
    extern context* ctx;

    struct value;
    using value_ref = std::shared_ptr<value>;

    using type_info = std::tuple<std::string, std::size_t, std::size_t, bool>;
    
    std::string type_of       (const type_info& info) {return std::get<0>(info);}
    std::size_t cache_size_of (const type_info& info) {return std::get<1>(info);}
    std::size_t file_size_of  (const type_info& info) {return std::get<2>(info);}
    bool        kept_opened_of(const type_info& info) {return std::get<3>(info);}

    inline std::ostream& operator<<(std::ostream& os, const type_info& info) {
      os << "{type = "                           << type_of       (info)
	 << ", cache_size = "                    << cache_size_of (info)
	 << ", file_size = "                     << file_size_of  (info)
	 << ", kept_opened = " << std::boolalpha << kept_opened_of(info)
	 << '}';
      return os;
    }

    struct timeline {
      timeline(const std::string& tl);
      ~timeline();
    };

    struct name_space {
      name_space(const std::string& tl);
      ~name_space();
    };
    
    struct offset {
      int value = 0;
      bool operator< (const offset& arg) const {return value  < arg.value;}
      bool operator==(const offset& arg) const {return value == arg.value;}
      bool operator!=(const offset& arg) const {return value != arg.value;}
      offset() = default;
      offset(int v) : value(v) {}
    };
    offset operator""_step  (unsigned long long int v) {return {int(v)};}

  
    using step = std::variant<unsigned int, offset>;
  

    struct value_key {
      std::string timeline;
      std::string name;
      value_key(const std::string& timeline, const std::string& name) : timeline(timeline), name(name) {}
      bool operator <(const value_key& k2) const {return std::make_tuple(timeline, name)  < std::make_tuple(k2.timeline, k2.name);}
      bool operator==(const value_key& k2) const {return std::make_tuple(timeline, name) == std::make_tuple(k2.timeline, k2.name);}

    };
  
    struct value_at_key {
      value_key   key;
      step        date;
    
      value_at_key(const std::string& timeline, const std::string& name, int date) : key(timeline, name), date(date) {}
      value_at_key(const std::string& timeline, const std::string& name, const offset& date) : key(timeline, name), date(date) {}
      value_at_key(const value_at_key&) = default;
      value_at_key& operator=(const value_at_key&) = default;
    
      bool operator <(const value_at_key& v2) const {return std::make_tuple(key, date)  < std::make_tuple(v2.key, v2.date);}
      bool operator==(const value_at_key& v2) const {return std::make_tuple(key, date) == std::make_tuple(v2.key, v2.date);}
    };
  

    std::ostream& operator<<(std::ostream& os, const step&         s);
    std::ostream& operator<<(std::ostream& os, const value_key&    k);
    std::ostream& operator<<(std::ostream& os, const value_at_key& k);

    namespace kwd {
      struct param {
	std::string name;
	double value;
	param(const std::string name, double value) : name(name), value(value) {}
      };

      using parameters = std::map<std::string, double>;

      inline param use(const std::string name, double value) {return {name, value};}
      inline parameters& operator|(parameters& ps, const param& p) {
	ps[p.name] = p.value;
	return ps;
      }

    }
      
    inline kwd::parameters& operator,(kwd::parameters& ps, const kwd::param& p) {
      ps[p.name] = p.value;
      return ps;
    }
    inline kwd::parameters& operator|(kwd::parameters& ps, const kwd::parameters& p) {
      for(auto& kv : p) ps | kwd::param(kv.first, kv.second);
      return ps;
    }
    inline kwd::parameters& operator,(kwd::parameters& ps, const kwd::parameters& p) {
      for(auto& kv : p) ps | kwd::param(kv.first, kv.second);
      return ps;
    }
    
    namespace kwd {
      struct data {
	value_at_key id;
	data(): id("", "", 0) {}
	data(const data&) = default;
	data& operator=(const data&) = default;

	data(const std::string& timeline, const std::string& name, const offset& date);
	data(const char*        timeline, const std::string& name, const offset& date) : data(std::string(timeline), name, date) {}
	data(const std::string& timeline, const char*        name, const offset& date) : data(timeline, std::string(name), date) {}
	data(const char*        timeline, const char*        name, const offset& date) : data(std::string(timeline), std::string(name), date) {}
      
	data(const std::string& timeline, const std::string& name, int date);
	data(const char*        timeline, const std::string& name, int date) : data(std::string(timeline), name, date) {}
	data(const std::string& timeline, const char*        name, int date) : data(timeline, std::string(name), date) {}
	data(const char*        timeline, const char*        name, int date) : data(std::string(timeline), std::string(name), date) {}
      
	data(const std::string& timeline, const std::string& name) : data(timeline, name, 0_step) {}
	data(const char*        timeline, const std::string& name) : data(timeline, name, 0_step) {}
	data(const std::string& timeline, const char*        name) : data(timeline, name, 0_step) {}
	data(const char*        timeline, const char*        name) : data(timeline, name, 0_step) {}
      
	data(const std::string& name, const offset& date);
	data(const std::string& name, int date);

	data(const char* name, const offset& date) : data(std::string(name), date) {}
	data(const char* name, int date)           : data(std::string(name), date) {}
      
	data(const std::string& name) : data(name, 0_step) {}
	data(const char* name)        : data(name, 0_step) {}

	void rename(const value_key& from, const value_key& to) {if(id.key == from) id.key = to;}
      };
    
      std::ostream& operator<<(std::ostream& os, const param& p);
      std::ostream& operator<<(std::ostream& os, const data& d);

      void type(data d,
		const std::string& t,
		std::size_t cache_size,
		std::size_t file_size,
		bool kept_opened);
      void type(const std::vector<data> range,
		const std::string& t,
		std::size_t cache_size,
		std::size_t file_size,
		bool kept_opened);
      void type(data d,
		const std::string& t,
		bool kept_opened);
      void type(const std::vector<data> range,
		const std::string& t,
		bool kept_opened);
      std::string type_of(data d);

      std::string var_path(const std::string& root_dir, data d) {
	std::string res = "";
	if(root_dir.size() == 0)                    res += "./";
	else if(root_dir[root_dir.size()-1] != '/') res += root_dir + '/';
	else                                        res += root_dir;
	res += d.id.key.timeline;
	res += d.id.key.name;
	res += ".var";
	return res;
      }

      inline data var(const std::string& timeline, const std::string& name) {
	return {timeline, name};
      }
      /**
       * This modifies the variable name in order to add a number.
       */
      inline data ith(data d, unsigned int i) {d.id.key.name += std::string("-") + std::to_string(i); return d;}
      std::vector<data> ith(const data& d, unsigned int begin, unsigned int end);
      std::vector<data> ith(const std::vector<data>& d, unsigned int i);

      /**
       * This defines the time step for a variable.
       */
      inline data at(data d, unsigned int s) {d.id.date = s; return d;}
      std::vector<data> at(const data& d, unsigned int begin, unsigned int end);
      std::vector<data> at(const std::vector<data>& d, unsigned int s);

      /**
       * This defines a time offset for a variable.
       */
      inline data shift(data d, int off) {d.id.date = offset(off); return d;}
      std::vector<data> shift(const data& d, int begin, int end);
      std::vector<data> shift(const std::vector<data>& d, int off);

      /**
       * This is shift(-1).
       */
      inline data prev(data d) {return shift(d, -1);}
      std::vector<data> prev(const std::vector<data>& d) {return shift(d, -1);}
      
      /**
       * Only timeline and name is concerned.
       */
      void rename(const data& from, const data& to);
    }
  

    struct update {
      unsigned int node_idf = 0;
      std::string name;
      kwd::data dest;
      std::vector<kwd::data> args;
      std::vector<kwd::param> params;

      update() = default;
    
      update(const std::string& name, const std::vector<kwd::data>& args)
	: node_idf(0), name(name), dest(), args(args), params() {}

      update& operator|(const kwd::param&  p)  {params.push_back(p);                                        return *this;}
      update& operator,(const kwd::param&  p)  {params.push_back(p);                                        return *this;}
      update& operator|(const kwd::parameters& ps) {for(auto& kv : ps) params.push_back({kv.first, kv.second}); return *this;}
      update& operator,(const kwd::parameters& ps) {for(auto& kv : ps) params.push_back({kv.first, kv.second}); return *this;}

      void rename(const value_key& from, const value_key& to) {
	dest.rename(from, to);
	for(auto& d: args) d.rename(from, to);
      }

      bool warn_about_walltime() const {
	bool warning = true;
	for(auto& p : params)
	  if(p.name == "walltime") {
	    warning = (p.value == 0);
	    break;
	  }
	return warning;
      }
    };
    
    std::ostream& operator<<(std::ostream& os, const update& u);
  
    struct context {
    private:
      struct Indent {
	unsigned int i = 0;
	void operator++(int) {++i;};
	void operator--(int) {--i;};
	std::string operator()() const {return std::string(2*i, ' ');}
	void clear() {i = 0;}
      };
      mutable Indent indent;
      bool user_argv_error = false;

    public:

      std::vector<std::string> argv;
      std::vector<std::string> user_argv;
    
      std::stack<std::string> timelines;
      std::stack<std::string> name_spaces;
      std::set<value_at_key> declared_instances;
      std::set<value_key> declared_values;
      std::map<value_key, type_info> declared_types;
      std::map<value_at_key, update> updates;
      std::map<value_at_key, update> inits;

      template<typename ArgIt>
      context(ArgIt begin, ArgIt end);
      context(int argc, char** argv);
      ~context();
      std::string current_timeline() const ;
      std::string varname(const std::string& name) const;
      kwd::data operator()(const kwd::data& data);
      void declare_type(value_key v,
			const std::string& t,
			std::size_t cache_size,
			std::size_t file_size,
			bool kept_opened);
      std::string type_of(value_key v) const;
      update& add_update(value_at_key v, update& u);
      update& add_init(value_at_key v, update& u);
      void rename(const value_key& from, const value_key& to);

      void check_walltimes() const;
      void check_orphan_inits() const;
      void handle_answer(boost::asio::ip::tcp::iostream& socket);
      void send(const std::string& hostname, int port);
      void print_graph(std::ostream& file, std::map<value_at_key, update>& rules, bool full_names);
      void notify_user_argv_error() {user_argv_error = true;}
    };

    namespace fx {
      inline update converge(const std::vector<kwd::data>& r) {
	std::vector<kwd::data> args;
	for(auto& d : r)
	  args.push_back((*ctx)(d));
	return {"converge", args};
      }
      inline update average(const std::vector<kwd::data>& r) {
	std::vector<kwd::data> args;
	for(auto& d : r)
	  args.push_back((*ctx)(d));
	return {"average", args};
      }
      inline update context_merge(const std::vector<kwd::data>& r) {return average(r);}
      
      inline update clear() {return {"clear", {}};}
      inline update random() {return {"random", {}};}
      
      inline update copy(const kwd::data& v) {return {"copy", {(*ctx)(v)}};}
      inline update merge(const kwd::data& v1, const kwd::data& v2) {return {"merge", {(*ctx)(v1), (*ctx)(v2)}};}
      inline update toward_bmu(const kwd::data& v) {return {"toward_bmu", {(*ctx)(v)}};}
      inline update match_triangle(const kwd::data& v1, const kwd::data& v2) {return {"match-triangle", {(*ctx)(v1), (*ctx)(v2)}};}
      inline update match_gaussian(const kwd::data& v1, const kwd::data& v2) {return {"match-gaussian", {(*ctx)(v1), (*ctx)(v2)}};}
      inline update learn_triangle(const kwd::data& v1, const kwd::data& v2, const kwd::data& v3) {return {"learn-triangle", {(*ctx)(v1), (*ctx)(v2), (*ctx)(v3)}};}
      inline update learn_gaussian(const kwd::data& v1, const kwd::data& v2, const kwd::data& v3) {return {"learn-gaussian", {(*ctx)(v1), (*ctx)(v2), (*ctx)(v3)}};}
      inline update argmax(const kwd::data& v) {return {"argmax", {(*ctx)(v)}};}
      inline update conv_argmax(const kwd::data& v) {return {"conv-argmax", {(*ctx)(v)}};}
      inline update toward_argmax(const kwd::data& v1, const kwd::data& v2) {return {"toward-argmax", {(*ctx)(v1), (*ctx)(v2)}};}
      inline update toward_conv_argmax(const kwd::data& v1, const kwd::data& v2) {return {"toward-conv-argmax", {(*ctx)(v1), (*ctx)(v2)}};}
    }
    
    std::pair<std::string, std::string> split_name(const std::string& name);

    update& operator<=(const kwd::data& v, const update& u);
    update& operator<<(const kwd::data& v, const update& u);
    void check_res(const kwd::data& v);
    void check_arg(const kwd::data& res, const kwd::data& arg);
    void check_args(const kwd::data& res, const update& u);
    void check_update(const kwd::data& res, const update& u);
  }
}




