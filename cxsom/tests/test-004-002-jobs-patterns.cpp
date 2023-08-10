
#define cxsomLOG
#include <cxsom-server.hpp>

// This is used in case of some macros are set. You can remove this
// lines otherwise... or keep it.
cxsom::Tick*    cxsom::ticker  = new cxsom::Tick();
cxsom::Log*     cxsom::logger  = new cxsom::Log();
cxsom::Monitor* cxsom::monitor = new cxsom::Monitor();

#include <random>

#include <filesystem>
namespace fs = std::filesystem;

int main(int, char**) {
  std::random_device rd;
  cxsom::data::Center data_center(fs::current_path() / "tmp");
  
  cxsom::jobs::UpdateFactory update_factory;
  cxsom::jobs::fill(update_factory);
  
  cxsom::jobs::TypeChecker type_checker;
  cxsom::jobs::fill(type_checker);
  
  cxsom::jobs::Center jobs_center(rd, update_factory, type_checker, data_center);


  data_center.check_all();

  jobs_center += cxsom::jobs::pattern::make({"main", "A"}, {"average", {{"main", "A", -1_relative}, {"main", "B", -1_relative}, {"main", "C", -1_relative}, {"main", "D", -1_relative}}, {}}, 100);
  jobs_center += cxsom::jobs::pattern::make({"main", "B"}, {"random", {}, {}}, 100);
  jobs_center += cxsom::jobs::pattern::make({"main", "C"}, {"random", {}, {}}, 100);
  jobs_center += cxsom::jobs::pattern::make({"main", "D"}, {"random", {}, {}}, 100);
  
  bool has_work = true;
  while(has_work)
    if(auto job = jobs_center.get_one(); job) job();
    else has_work = false;

  return 0;
}
