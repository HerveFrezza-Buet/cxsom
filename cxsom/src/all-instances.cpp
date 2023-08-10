#include <cxsom-server.hpp>

// This is used in case of some macros are set. You can remove this
// lines otherwise... or keep it.
cxsom::Tick*     cxsom::ticker  = nullptr;
cxsom::Log*      cxsom::logger  = nullptr;
cxsom::Monitor*  cxsom::monitor = nullptr;

#include <set>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <utility>

#include <filesystem>
namespace fs = std::filesystem;


template<typename It>
std::pair<std::size_t, std::size_t> max_name_length(std::size_t prefix_length, It begin, It end) {
  std::size_t m1 = 0;
  std::size_t m2 = 0;
  for(auto it = begin; it != end; ++it) {
    auto [s1, s2] = cxsom::symbol::parse::split_varpath(prefix_length, *it);
    if(s1.size() > m1) m1 = s1.size();
    if(s2.size() > m2) m2 = s2.size();
  }
  return {m1, m2};
}

int main(int argc, char* argv[]) {
  if(argc != 2) {
    std::cout << "Usage : " << argv[0] << " <cxsom-simulation-rootdir>" << std::endl;
    return 0;
  }

  std::string root_dir = argv[1];

  std::set<fs::path> var_paths;
  for(auto& elem: fs::recursive_directory_iterator(root_dir))
    if(auto p = elem.path(); p.extension() == ".var")
      var_paths.insert(p);
  
  auto prefix_length = cxsom::symbol::parse::root_dir_length(root_dir);
  
  auto [tl_width, name_width] = max_name_length(prefix_length, var_paths.begin(), var_paths.end());
  std::size_t type_width       = 0;
  std::size_t file_size_width  = 0;
  std::size_t cache_size_width = 0;
  std::size_t inf_time_width   = 0;
  std::size_t sup_time_width   = 0;

  std::vector<cxsom::data::File> files;
  std::vector<std::pair<cxsom::symbol::Variable, std::string>> errors;
  for(auto& p : var_paths) {
    auto [tl, name] = cxsom::symbol::parse::split_varpath(prefix_length, p);
    if(tl == "")
      throw std::runtime_error("No timeline here.");

    auto& file = files.emplace_back(root_dir, cxsom::symbol::Variable(tl, name));
    try {
      file.realize(nullptr, std::nullopt, std::nullopt, false);
      if(auto l = file.type_as_string().size();                 l > type_width)       type_width       = l;
      if(auto l = std::to_string(file.get_file_size()).size();  l > file_size_width)  file_size_width  = l;
      if(auto l = std::to_string(file.get_cache_size()).size(); l > cache_size_width) cache_size_width = l;
    }
    catch(std::exception& e) {
      errors.push_back({{tl, name}, std::string("realization raised an exception: ") + e.what()});
    }

    {
      auto [inf, sup] = file.get_time_range();
      if(inf != sup)
	if(inf != cxsom::data::File::no_time() && std::to_string(inf).size() > inf_time_width) inf_time_width = std::to_string(inf).size();
      if(sup != cxsom::data::File::no_time() && std::to_string(sup).size() > sup_time_width) sup_time_width = std::to_string(sup).size();
    }
  }
  

  for(auto& file : files)
    if(file) {
      try {
	file.pretty_print(std::cout,
			  tl_width, name_width,
			  type_width,
			  file_size_width, cache_size_width,
			  inf_time_width, sup_time_width);
      }
      catch(std::exception& e) {
	std::cout << "\e[91m \u26A0" << std::endl;
	errors.push_back({file, std::string("Printing failed, the file may be (temporarily) corrupted: ") + e.what()});
      }
    }

  std::size_t max_timeline_length = 0;
  std::size_t max_name_length     = 0;
  for(auto& error : errors) {
    max_timeline_length = std::max(max_timeline_length, error.first.timeline.size());
    max_name_length     = std::max(max_name_length,     error.first.name.size()    );
  }
  for(auto& error : errors)
    std::cout << "\e[91mError : [" 
	      << std::setw(max_timeline_length) << std::left << error.first.timeline << ", "
	      << std::setw(max_name_length)     << std::left << error.first.name << "]: "
	      << error.second
	      << "\e[0m"<< std::endl;
  
  return 0;
}

