
#define cxsomLOG
#include <cxsom-server.hpp>

// This is used in case of some macros are set. You can remove this
// lines otherwise... or keep it.
cxsom::Tick* cxsom::ticker = new cxsom::Tick();
cxsom::Log*  cxsom::logger = new cxsom::Log();

#include <random>

#include <filesystem>
namespace fs = std::filesystem;

int main(int, char**) {
  std::random_device rd;
  cxsom::data::Center data_center(fs::current_path() / "tmp");
  cxsom::jobs::Center jobs_center(rd, data_center);


  data_center.check_all();

  jobs_center += cxsom::jobs::pattern::make({"main", "A"}, {cxsom::jobs::Operation::Average, {{"main", "A", -1}, {"main", "B", -1}, {"main", "C", -1}, {"main", "D", -1}}, {}}, 100);
  jobs_center += cxsom::jobs::pattern::make({"main", "B"}, {cxsom::jobs::Operation::Random, {}, {}}, 100);
  jobs_center += cxsom::jobs::pattern::make({"main", "C"}, {cxsom::jobs::Operation::Random, {}, {}}, 100);
  jobs_center += cxsom::jobs::pattern::make({"main", "D"}, {cxsom::jobs::Operation::Random, {}, {}}, 100);
  
  bool has_work = true;
  while(has_work)
    if(auto job = jobs_center.get_one(); job) job();
    else has_work = false;

  return 0;
}
