#pragma once


/**
 * @example example-002-001-rules-basics.cpp
 * @example example-002-002-rules-all.cpp
 * @example example-002-003-namings.cpp
 * @example example-002-004-relaxation.cpp
 * @example example-002-005-ping.cpp
 * @example example-003-001-som1d-R-map.cpp
 * @example example-003-002-som1d-R2-map.cpp
 * @example example-003-003-som2d-RGB-map.cpp
 * @example example-004-001-cxsom-circle-maps.cpp
 * @example example-004-002-cxsom-recsom-maps.cpp
 */


#include <optional>
#include <typeinfo>

#include <cxsomRuleDefs.hpp>
#include <cxsomProtocolClient.hpp>



inline cxsom::rules::update&  cxsom::rules::operator<=(const cxsom::rules::kwd::data& v, const cxsom::rules::update& u) {
  check_update(v, u);
  auto U = u;
  U.dest = (*cxsom::rules::ctx)(v);
  return cxsom::rules::ctx->add_init(U.dest.id, U);
}

inline cxsom::rules::update&  cxsom::rules::operator<<(const cxsom::rules::kwd::data& v, const cxsom::rules::update& u) {
  check_update(v, u);
  auto U = u;
  U.dest = (*cxsom::rules::ctx)(v);
  return cxsom::rules::ctx->add_update(U.dest.id, U);
}

inline void cxsom::rules::check_update(const cxsom::rules::kwd::data& res, const cxsom::rules::update& u) {
  check_res(res);
  check_args(res, u);
}

inline void cxsom::rules::check_args(const cxsom::rules::kwd::data& res, const cxsom::rules::update& u) {
  if(std::holds_alternative<unsigned int>(res.id.date))
    for(auto& arg : u.args)
      check_arg(res, arg);
}

inline void cxsom::rules::check_arg(const cxsom::rules::kwd::data& res, const kwd::data& arg) {
  if(std::holds_alternative<offset>(arg.id.date)) {
    std::ostringstream ostr;
    ostr << "Update of instance " << res << " (which is not a pattern) requires only instance argument. Pattern argument " << arg << " found.";
    throw error::bad_update(ostr.str());
  }
}

inline void cxsom::rules::check_res(const cxsom::rules::kwd::data& v) {
  if(std::holds_alternative<offset>(v.id.date) && std::get<offset>(v.id.date).value != 0) {
    std::ostringstream ostr;
    ostr << "Update pattern can only concern non-shifted variables. Updating " << v << " is thus invalid."; 
    throw error::bad_update(ostr.str());
  }
}
  
inline std::vector<cxsom::rules::kwd::data> cxsom::rules::kwd::ith(const data& d, unsigned int begin, unsigned int end) {
  std::vector<cxsom::rules::kwd::data> res;
  auto out = std::back_inserter(res);
  unsigned int i = begin;
  while(i != end) *(out++) = ith(d, i++);
  return res;
}

inline std::vector<cxsom::rules::kwd::data> ith(const std::vector<cxsom::rules::kwd::data>& d, unsigned int i) {
  std::vector<cxsom::rules::kwd::data> res;
  auto out = std::back_inserter(res);
  for(auto& data : d) *(out++) = ith(data, i);
  return res;
}

inline std::vector<cxsom::rules::kwd::data> cxsom::rules::kwd::at(const data& d, unsigned int begin, unsigned int end) {
  std::vector<cxsom::rules::kwd::data> res;
  auto out = std::back_inserter(res);
  unsigned int i = begin;
  while(i != end) *(out++) = at(d, i++);
  return res;
}

inline std::vector<cxsom::rules::kwd::data> cxsom::rules::kwd::at(const std::vector<cxsom::rules::kwd::data>& d, unsigned int s) {
  std::vector<cxsom::rules::kwd::data> res;
  auto out = std::back_inserter(res);
  for(auto& data : d) *(out++) = at(data, s);
  return res;
}

inline std::vector<cxsom::rules::kwd::data> cxsom::rules::kwd::shift(const data& d, int begin, int end) {
  std::vector<cxsom::rules::kwd::data> res;
  auto out = std::back_inserter(res);
  int i = begin;
  while(i != end) *(out++) = shift(d, i++);
  return res;
}


inline std::vector<cxsom::rules::kwd::data> cxsom::rules::kwd::shift(const std::vector<cxsom::rules::kwd::data>& d, int off) {
  std::vector<cxsom::rules::kwd::data> res;
  auto out = std::back_inserter(res);
  for(auto& data : d) *(out++) = shift(data, off);
  return res;
}

inline void cxsom::rules::kwd::type(const std::vector<data> range,
				    const std::string& t,
				    std::size_t cache_size,
				    std::size_t file_size,
				    bool kept_opened) {
  for(auto& d : range) type(d, t, cache_size, file_size, kept_opened);
}

inline void cxsom::rules::kwd::type(data d, const std::string& t,
				    std::size_t cache_size,
				    std::size_t file_size,
				    bool kept_opened) {
  ctx->declare_type(d.id.key, t, cache_size, file_size, kept_opened);
}

inline void cxsom::rules::kwd::type(const std::vector<data> range,
				    const std::string& t,
				    bool kept_opened) {
  type(range, t, 0, 0, kept_opened);
}

inline void cxsom::rules::kwd::type(data d,
				    const std::string& t,
				    bool kept_opened) {
  type(d, t, 0, 0, kept_opened);
}

inline std::string cxsom::rules::kwd::type_of(data d) {
  return ctx->type_of(d.id.key);
}

inline void cxsom::rules::kwd::rename(const data& from, const data& to) {
  ctx->rename(from.id.key, to.id.key);
}
  
inline void cxsom::rules::context::rename(const value_key& from, const value_key& to) {
  bool from_in_inits   = false; 
  bool from_in_updates = false; 
  bool to_in_inits     = false; 
  bool to_in_updates   = false;

  for(auto kv : inits) {
    from_in_inits |= (from == kv.first.key);
    to_in_inits   |= (to   == kv.first.key);
  }

  for(auto kv : updates) {
    from_in_updates |= (from == kv.first.key);
    to_in_updates   |= (to   == kv.first.key);
  }

  if((from_in_inits || from_in_updates) && (to_in_inits || to_in_updates)) {
    std::ostringstream ostr;
    ostr << "Renaming from " << from << " to " << to << "creates double updates/inits.";
    throw error::invalid_rename(ostr.str());
  }

  {
    declared_values.erase(from);
    declared_values.insert(to);

    std::set<value_at_key> to_rename;
    for(auto& v : declared_instances)
      if(v.key == from)
	to_rename.insert(v);
    for(auto v : to_rename) {
      declared_instances.erase(v);
      v.key = to;
      declared_instances.insert(v);
    }
  }

  if(auto it = declared_types.find(from); it != declared_types.end()) {
    auto t = it->second;
    declared_types.erase(it);
    declared_types[to] = t;
  }
  
  {
    auto tmp = updates;
    updates.clear();
    for(std::pair<value_at_key, update> kv: tmp) {
      if(kv.first.key == from) kv.first.key = to;
      kv.second.rename(from, to);
      updates[kv.first] = kv.second;
    }
  }

  {
    auto tmp = inits;
    inits.clear();
    for(std::pair<value_at_key, update> kv: tmp) {
      if(kv.first.key == from) kv.first.key = to;
      kv.second.rename(from, to);
      inits[kv.first] = kv.second;
    }
  }

  {
    auto tmp = inits;
    inits.clear();
    for(std::pair<value_at_key, update> kv: tmp) {
      if(kv.first.key == from) kv.first.key = to;
      kv.second.rename(from, to);
      inits[kv.first] = kv.second;
    }
  }
	
}
  
inline cxsom::rules::kwd::data cxsom::rules::context::operator()(const cxsom::rules::kwd::data& d) {
  declared_instances.insert(d.id);
  declared_values.insert(d.id.key);
  return d;
}

inline std::string cxsom::rules::context::type_of(value_key v) const {
  if(auto it = declared_types.find(v); it != declared_types.end()) 
    return cxsom::rules::type_of(it->second);
  else {
      std::ostringstream ostr;
      ostr << "Asking for type of variable " << v << " while it is not known yet." << std::endl;
      throw error::unknown_variable(ostr.str());
  }
}

inline void cxsom::rules::context::declare_type(value_key v,
						const std::string& t,
						std::size_t cache_size,
						std::size_t file_size,
						bool kept_opened) {
  type_info info(t, cache_size, file_size, kept_opened);
  if(auto it = declared_types.find(v); it != declared_types.end()) {
    if(it->second != info) {
      std::ostringstream ostr;
      ostr << "Variable " << v << " is already declared as having type " << it->second << " while declaring it with type " << info << '.';
      throw error::redeclare_type(ostr.str());
    }
  }
  else
    declared_types[v] = info;
}

void cxsom::rules::context::check_walltimes() const {
  for(auto& kv : updates) {
    std::optional<update> init;
    if(auto it = inits.find(kv.first); it != inits.end()) init = it->second;
    if(std::holds_alternative<offset>(kv.first.date)) {
      if(kv.second.warn_about_walltime())
	std::cout << "Warning : Update " << kv.first << " has null walltime." << std::endl;
    }
  }
}

void cxsom::rules::context::check_orphan_inits() const {
  for(auto& kv : inits)
    if(auto it = updates.find(kv.first); it == updates.end()) {
      std::ostringstream ostr;
      ostr << "The init update for " << kv.first << " is orphan, no usual update is provided.";
      throw cxsom::rules::error::bad_update(ostr.str());
    }
}

inline cxsom::rules::update& cxsom::rules::context::add_update(cxsom::rules::value_at_key v, cxsom::rules::update& u) {
  if(updates.find(v) != updates.end()) {
    std::ostringstream ostr;
    ostr << "A previous update for " << v << " already exists.";
    throw cxsom::rules::error::double_update(ostr.str());
  }
  return updates[v] = u;
}

inline cxsom::rules::update& cxsom::rules::context::add_init(cxsom::rules::value_at_key v, cxsom::rules::update& u) {
  if(inits.find(v) != inits.end()) {
    std::ostringstream ostr;
    ostr << "A previous init for " << v << " already exists.";
    throw cxsom::rules::error::double_update(ostr.str());
  }
  return inits[v] = u;
}

  
inline cxsom::rules::kwd::data::data(const std::string& timeline, const std::string& name, const offset& date) : id(timeline, ctx->varname(name), date) {}
inline cxsom::rules::kwd::data::data(const std::string& timeline, const std::string& name, int date) : id(timeline, ctx->varname(name), date) {}

inline cxsom::rules::kwd::data::data(const std::string& name, int date) : data(ctx->current_timeline(), name, date) {}
inline cxsom::rules::kwd::data::data(const std::string& name, const offset& date) : data(ctx->current_timeline(), name, date) {}

inline cxsom::rules::timeline::timeline(const std::string& tl) {ctx->timelines.push(tl);}
inline cxsom::rules::timeline::~timeline()                     {ctx->timelines.pop();}

inline cxsom::rules::name_space::name_space(const std::string& tl) {ctx->name_spaces.push(tl);}
inline cxsom::rules::name_space::~name_space()                     {ctx->name_spaces.pop();}


inline std::string cxsom::rules::context::current_timeline() const {return timelines.top();}

inline std::string cxsom::rules::context::varname(const std::string& name) const {
  if(name == "")
    throw std::runtime_error("Bad empty name variable");

  if(name[0] == '/')
    return name;
  
  std::string res = "/";
  std::stack<std::string> top_to_bottom = name_spaces;
  std::stack<std::string> bottom_to_top;
  while(!top_to_bottom.empty()) {
    bottom_to_top.push(top_to_bottom.top());
    top_to_bottom.pop();
  }
  
  while(!bottom_to_top.empty()) {
    res += (bottom_to_top.top() + "/");
    bottom_to_top.pop();
  }
  res += name;
  return res;
}


inline cxsom::rules::offset operator-(const cxsom::rules::offset& o) {
  return {-o.value};
}
  
inline cxsom::rules::offset operator+(const cxsom::rules::offset& o, int i) {
  return {o.value + i};
}
  
inline cxsom::rules::offset operator-(const cxsom::rules::offset& o, int i) {
  return {o.value - i};
}

inline bool operator<(const cxsom::rules::step& s1, const cxsom::rules::step& s2) {
  if(std::holds_alternative<unsigned int>(s1))
    if(std::holds_alternative<unsigned int>(s2))
      return std::get<unsigned int>(s1) < std::get<unsigned int>(s2);
    else
      return true;
  else
    if(std::holds_alternative<unsigned int>(s2))
      return false;
    else
      return std::get<cxsom::rules::offset>(s1) < std::get<cxsom::rules::offset>(s2);
}

inline std::pair<std::string, std::string> cxsom::rules::split_name(const std::string& name) {
  if(name == "") return {"", ""};

  auto it = name.end() - 1;
  while((it != name.begin()) && (*it != '/')) --it;
  std::string prefix, suffix;
  std::copy(name.begin(), it, std::back_inserter(prefix));
  std::copy(it+1, name.end(), std::back_inserter(suffix));

  return {prefix, suffix};
}





///////////////////
//               //
// Serialization //
//               //
///////////////////


inline std::ostream& cxsom::rules::operator<<(std::ostream& os, const cxsom::rules::step& s) {
  if(std::holds_alternative<unsigned int>(s))
    os << std::get<unsigned int>(s);
  else 
    os << '@' << std::get<cxsom::rules::offset>(s).value;
  return os;
}

inline std::ostream& cxsom::rules::operator<<(std::ostream& os, const cxsom::rules::value_key& k) {
  os << '[' << k.timeline << ", " << k.name << ']';
  return os;
}
  
inline std::ostream& cxsom::rules::operator<<(std::ostream& os, const cxsom::rules::value_at_key& k) {
  os << '[' << k.key.timeline << ", " << k.key.name << ", " << k.date << ']';
  return os;
}

inline std::ostream& cxsom::rules::kwd::operator<<(std::ostream& os, const cxsom::rules::kwd::param& p) {
  os << p.name << " = " << p.value;
  return os;
}

inline std::ostream& cxsom::rules::kwd::operator<<(std::ostream& os, const cxsom::rules::kwd::data& d) {
  os << d.id;
  return os;
}
inline std::ostream& cxsom::rules::operator<<(std::ostream& os, const cxsom::rules::update& u) {
  os << u.dest << " <- " << u.name << '(';
  auto it = u.args.begin();
  if(it != u.args.end())    os         << *(it++);
  while(it != u.args.end()) os << ", " << *(it++);
  os << ") {";

  auto pit = u.params.begin();
  if(pit != u.params.end())    os         << *(pit++);
  while(pit != u.params.end()) os << ", " << *(pit++);
  os << '}';
  return os;
}



/////////////
//         //
// Context //
//         //
/////////////




inline void cxsom::rules::context::print_graph(std::ostream& file, std::map<value_at_key, update>& rules, bool full_names) {
  std::map<value_at_key, unsigned int> value_idf;
  std::map<std::string, unsigned int> time_idf;
    
  unsigned int rule_idf = 0;
  unsigned int data_idf = 0;
  unsigned int timeline_idf = 0;
  unsigned int prefix_idf = 0;
    
  for(auto& kv : rules) {
    kv.second.node_idf = rule_idf++;
    if(value_idf.find(kv.first) == value_idf.end()) value_idf[kv.first] = data_idf++;
    if(time_idf.find(kv.first.key.timeline) == time_idf.end()) time_idf[kv.first.key.timeline] = timeline_idf++;
    for(auto& d : kv.second.args) {
      if(value_idf.find(d.id) == value_idf.end()) value_idf[d.id] = data_idf++;
      if(time_idf.find(d.id.key.timeline) == time_idf.end()) time_idf[d.id.key.timeline] = timeline_idf++;
    }
  }
  indent.clear();
  file << indent() << "digraph All {" << std::endl;
  indent++;
  for(auto& kv : time_idf) {
    file << std::endl << indent() << "subgraph cluster_t" << kv.second << " {" << std::endl;
    indent++;
    file << indent() << "label = <<font point-size=\"30\">timeline <b>" << kv.first << "</b></font>>" << std::endl
	 << indent() << "fillcolor = \"#fff0ff\"" << std::endl
	 << indent() << "style = \"filled\"" << std::endl
	 << indent() << "penwidth = 0" << std::endl;

      
    std::map<std::string, unsigned int> prefixes;
      
    for(auto& kkvv : value_idf) 
      if(kkvv.first.key.timeline == kv.first) {
	auto p = split_name(kkvv.first.key.name).first;
	if(prefixes.find(p) == prefixes.end()) prefixes[p] = prefix_idf++;
      }


    for(auto& pv : prefixes) {

      file << std::endl << indent() << "subgraph cluster_p" << pv.second << " {" << std::endl;
      indent++;
      file << indent() << "label = \"" << pv.first << "\"" << std::endl
	   << indent() << "fillcolor = \"#ffccff\"" << std::endl
	   << indent() << "style = \"filled\"" << std::endl
	   << indent() << "penwidth = 0" << std::endl;
      for(auto& kkvv : value_idf) 
	if((kkvv.first.key.timeline == kv.first)
	   && (split_name(kkvv.first.key.name).first == pv.first)) {
	  std::string label;
	  if(full_names)
	    label = std::string("(") + kkvv.first.key.timeline + ") " + kkvv.first.key.name;
	  else
	    label = split_name(kkvv.first.key.name).second;
	  std::string color = "#ccccff";
	  if(std::holds_alternative<unsigned int>(kkvv.first.date)) {
	    color = "#8888ff";
	    label += " @";
	    label += std::to_string(std::get<unsigned int>(kkvv.first.date));
	  }
	  else {
	    auto v = std::get<offset>(kkvv.first.date).value;
	    if(v < 0) {
	      label += " @now-";
	      label += std::to_string(-v);
	    }
	    else if(v > 0) {
	      label += " @now";
	      label += std::to_string(v);
	    }
	    if(v != 0)
	      color = "#ccccdd";
	  }
	  std::string style = "filled";
	  label = std::string("<b>")+label+"</b>";
	  file << indent() << 'n' << kkvv.second
	       << " [label=<" << label 
	       << "<br/><font point-size=\"8\">";
	  if(auto it = declared_types.find(kkvv.first.key); it == declared_types.end())
	    file << "???";
	  else {
	    std::string type_webname = "";
	    for(auto c : cxsom::rules::type_of(it->second))
	      switch(c) {
	      case '<': type_webname += "&lt;"; break;
	      case '>': type_webname += "&gt;"; break;
	      default:
		type_webname.push_back(c); break;
	      }
	    file << type_webname;
	  }
	  file << "</font>"
	       << ">, shape=box, fillcolor=\"" << color << "\", style=\"" << style << "\"]" << std::endl;
	}

      for(auto& kkvv : rules) 
	if((kkvv.first.key.timeline == kv.first)
	   && (split_name(kkvv.first.key.name).first == pv.first)) {
	  std::ostringstream ostr;
	  ostr << "<b>" + kkvv.second.name + "</b>";
	  for(auto& p : kkvv.second.params) 
	    ostr << "<br/><font point-size=\"10\">" << p.name << " = " << p.value << "</font>";
	  file << indent() << 'u' << kkvv.second.node_idf
	       << " [label=<" << ostr.str() << ">, shape=ellipse, fillcolor=\"#ffffaa\", style=\"filled\"]" << std::endl;
	}
      indent--;
      file << indent() << '}' << std::endl;;
    }
      
    indent--;
    file << indent() << "}" << std::endl;
  }

  for(auto& kv : rules) {
    file << indent() << 'u' << kv.second.node_idf << " -> n" << value_idf[kv.second.dest.id] << std::endl;
    for(auto& arg : kv.second.args)
      file << indent() << 'n' << value_idf[arg.id] << " -> u" << kv.second.node_idf << std::endl;
  }

  indent--;
  file << indent() << "}" << std::endl;
}



inline void cxsom::rules::context::handle_answer(boost::asio::ip::tcp::iostream& socket) {
  std::string line;
  std::getline(socket, line, '\n');
#ifdef cxsomDEBUG_PROTOCOL
  if(line != "ok") {
    std::cout << ">> error : " << line << std::endl;
    throw std::runtime_error("server processing failed");
  }
  else
    std::cout << ">> ok" << std::endl;
#else
  if(line != "ok") {
    std::cerr << line << std::endl;
    throw std::runtime_error("server processing failed");
  }
#endif
}

inline void cxsom::rules::context::send(const std::string& hostname, int port) {
  boost::asio::ip::tcp::iostream socket;
  socket.exceptions(std::ios::failbit | std::ios::badbit | std::ios::eofbit);
  try {
    socket.connect(hostname, std::to_string(port));

    for(auto& kv : declared_types) {
      protocol::write::type_declaration(socket, kv.first,
					cxsom::rules::type_of       (kv.second),
					cxsom::rules::cache_size_of (kv.second),
					cxsom::rules::file_size_of  (kv.second),
					cxsom::rules::kept_opened_of(kv.second));
      socket << std::flush;
      handle_answer(socket);
    }
    
    if(updates.size() > 0) {
      protocol::write::updates(socket, updates.size());
      socket << std::flush;
      for(auto& kv : updates) {
	std::optional<update> init;
	if(auto it = inits.find(kv.first); it != inits.end()) init = it->second;
	if(std::holds_alternative<unsigned int>(kv.first.date)) 
	  protocol::write::update(socket, kv.first, init, kv.second);
	else
	  protocol::write::pattern::update(socket, kv.first, init, kv.second);
	socket << std::flush;
	handle_answer(socket);
      }
    }
    
    socket.close();
  }		
  catch(std::exception& e) {
    std::cerr << "Exception caught : " << e.what() << " --> " << typeid(e).name()<< std::endl;
  }
}


template<typename ArgIt>
cxsom::rules::context::context(ArgIt begin, ArgIt end)  
  : argv(), user_argv(),
    timelines(),
    name_spaces(),
    declared_instances(),
    declared_values(),
    updates(),
    inits()
{
  ctx = this;
  auto out = std::back_inserter(this->argv);
  while(begin != end && std::string(*begin) != "--") *(out++) = *(begin++);
  if(begin != end && std::string(*begin) == "--") {
    ++begin;
    out = std::back_inserter(this->user_argv);
    while(begin != end) *(out++) = *(begin++);
  }
}

inline cxsom::rules::context::context(int argc, char** argv)
  : context(argv, argv + argc) {}

inline cxsom::rules::context::~context() {
  if(user_argv_error) return;
  
  check_orphan_inits();
  check_walltimes();
  if(argv.size() < 2) {
    std::cout << "Usage :" << std::endl
	      << "  " << argv[0] << " debug [-- ...]" << std::endl
	      << "  " << argv[0] << " graph <file-prefix> [-- ...]" << std::endl
	      << "  " << argv[0] << " graph-full <file-prefix> [-- ...]" << std::endl
	      << "  " << argv[0] << " send  <hostname> <port> [-- ...]" << std::endl
	      << std::endl
	      << "Arguments following -- are supplementary arguments for user-define purpose."
	      << std::endl;
    return;
  }

  if(argv[1] == "debug") {
    std::cout << "# Declared types:"      << std::endl; 
    for(auto vt : declared_types)            std::cout  << "  " << vt.first << " has type " << vt.second << std::endl;
    std::cout << std::endl << "# Declared instances:"  << std::endl; 
    for(auto d : declared_instances)         std::cout  << "  " << d << std::endl;
    std::cout << std::endl << "# Inits"   << std::endl;
    for(auto& kv : inits)                    std::cout  << "  " << kv.second << std::endl;
    std::cout << std::endl << "# Updates" << std::endl;
    for(auto& kv : updates)                  std::cout  << "  " << kv.second << std::endl;
  }
  else if(argv[1] == "graph" || argv[1] == "graph-full") {
    if(size(argv) < 3) {
      std::cout << "Usage :" << std::endl
		<< "  " << argv[0] << " [graph | graph-full] <file-prefix>" << std::endl;
      return;
    }
    bool full = (argv[1] == "graph-full");
    {
      auto filename = argv[2]+"-updates.dot";
      std::ofstream file(filename.c_str());
      print_graph(file, updates, full);
      std::cout << "file \"" << filename << "\" generated." << std::endl;
    }

    {
      auto filename = argv[2]+"-inits.dot";
      std::ofstream file(filename.c_str());
      print_graph(file, inits, full);
      std::cout << "file \"" << filename << "\" generated." << std::endl;
    }
  }
  else if(argv[1] == "send") {
    if(size(argv) < 4) {
      std::cout << "Usage :" << std::endl
		<< "  " << argv[0] << " send <hostname> <port>" << std::endl;
      return;
    }

    send(argv[2], std::stoi(argv[3]));
  }
  else {
    std::cout << "Invalid command \"" << argv[1] << "\", run without arguments to get help." << std::endl;
    return;
  }
}
