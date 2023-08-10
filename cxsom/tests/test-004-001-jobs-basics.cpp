
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

#define CACHE_SIZE     10
#define FILE_SIZE    1000
#define KEPT_OPENED  true

int main(int, char**) {
  std::random_device rd;
  cxsom::data::Center data_center(fs::current_path() / "tmp");
  
  cxsom::jobs::UpdateFactory update_factory;
  cxsom::jobs::fill(update_factory);
  
  cxsom::jobs::TypeChecker type_checker;
  cxsom::jobs::fill(type_checker);
  
  cxsom::jobs::Center jobs_center(rd, update_factory, type_checker, data_center);

  data_center.check({"main", "A"}, cxsom::type::make("Map1D<Scalar>=500"), CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
  data_center.check({"main", "B"}, cxsom::type::make("Map1D<Scalar>=500"), CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
  data_center.check({"main", "C"}, cxsom::type::make("Map1D<Scalar>=500"), CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
  data_center.check({"main", "D"}, cxsom::type::make("Map1D<Scalar>=500"), CACHE_SIZE, FILE_SIZE, KEPT_OPENED);

  jobs_center += cxsom::jobs::make({"main", "A", 0}, {"random", {}, {}});
  jobs_center += cxsom::jobs::make({"main", "B", 0}, {"random", {}, {}});
  jobs_center += cxsom::jobs::make({"main", "C", 0}, {"random", {}, {}});
  jobs_center += cxsom::jobs::make({"main", "D", 0}, {"random", {}, {}});

  jobs_center += cxsom::jobs::make({"main", "A", 1}, {"average", {{"main", "A", 0}, {"main", "B", 0}, {"main", "C", 0}, {"main", "D", 0}}, {}});
  jobs_center += cxsom::jobs::make({"main", "B", 1}, {"average", {{"main", "A", 1}, {"main", "B", 1}, {"main", "C", 1}, {"main", "D", 1}}, {}});
  jobs_center += cxsom::jobs::make({"main", "C", 1}, {"average", {{"main", "A", 1}, {"main", "B", 1}, {"main", "C", 1}, {"main", "D", 1}}, {}});
  jobs_center += cxsom::jobs::make({"main", "D", 1}, {"average", {{"main", "A", 1}, {"main", "B", 1}, {"main", "C", 1}, {"main", "D", 1}}, {}});
      
  bool has_work = true;
  while(has_work)
    if(auto job = jobs_center.get_one(); job) job();
    else has_work = false;

  return 0;
}

