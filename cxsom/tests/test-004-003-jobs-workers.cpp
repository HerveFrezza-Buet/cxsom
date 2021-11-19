


#include <cxsom-server.hpp>

// This is used in case of some macros are set. You can remove this
// lines otherwise... or keep it.
cxsom::Tick* cxsom::ticker = new cxsom::Tick();
cxsom::Log*  cxsom::logger = new cxsom::Log();

#include <random>
#include <thread>
#include <vector>
#include <iterator>
#include <chrono>
#include <iostream>

#include <filesystem>
namespace fs = std::filesystem;
using namespace std::chrono_literals;

#define NB_WORKERS      5
#define CACHE_SIZE     10
#define FILE_SIZE    1000
#define KEPT_OPENED  true

int main(int, char**) {
  std::random_device rd;
  cxsom::data::Center data_center(fs::current_path() / "tmp");
  cxsom::jobs::Center jobs_center(rd, data_center);

  data_center.check({"main", "A"}, cxsom::type::make("Map1D<Scalar>=500"), CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
  data_center.check({"main", "B"}, cxsom::type::make("Map1D<Scalar>=500"), CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
  data_center.check({"main", "C"}, cxsom::type::make("Map1D<Scalar>=500"), CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
  data_center.check({"main", "D"}, cxsom::type::make("Map1D<Scalar>=500"), CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
  
  data_center.check_all();

  std::vector<std::thread> workers;
  for(unsigned int i = 0; i < NB_WORKERS; ++i)
    workers.emplace_back([&jobs_center](){jobs_center.worker_thread();});

  std::cout << "Sleeping for 5s..." << std::endl;
  std::this_thread::sleep_for(5s);
  
  jobs_center.interaction_lock();
  std::cout << "Initializing A[0], A[1], A[2], B[0:3], C[0:3], D[0:3]..." << std::endl
	    << "... and setting pattern of A." << std::endl;
  jobs_center += cxsom::jobs::make({"main", "A", 0}, {cxsom::jobs::Operation::Clear, {}, {}});
  jobs_center += cxsom::jobs::make({"main", "A", 1}, {cxsom::jobs::Operation::Clear, {}, {}});
  jobs_center += cxsom::jobs::make({"main", "A", 2}, {cxsom::jobs::Operation::Clear, {}, {}});
  jobs_center += cxsom::jobs::pattern::make({"main", "A"}, {cxsom::jobs::Operation::Average, {{"main", "B", -1}, {"main", "C", -1}, {"main", "D", -1}}, {}}, 100);
  
  jobs_center += cxsom::jobs::pattern::make({"main", "B"}, {cxsom::jobs::Operation::Random, {}, {}}, 2);
  jobs_center += cxsom::jobs::pattern::make({"main", "C"}, {cxsom::jobs::Operation::Random, {}, {}}, 2);
  jobs_center += cxsom::jobs::pattern::make({"main", "D"}, {cxsom::jobs::Operation::Random, {}, {}}, 2);
  jobs_center.interaction_release();
  
  std::cout << "Sleeping for 5s..." << std::endl;
  std::this_thread::sleep_for(5s);
  
  jobs_center.interaction_lock();
  std::cout << "Setting patterns for B, C, D until 10" << std::endl;
  jobs_center += cxsom::jobs::pattern::make({"main", "B"}, {cxsom::jobs::Operation::Average, {{"main", "B", -1}, {"main", "B", -2}, {"main", "B", -3}}, {}}, 10);
  jobs_center += cxsom::jobs::pattern::make({"main", "C"}, {cxsom::jobs::Operation::Average, {{"main", "C", -1}, {"main", "C", -2}, {"main", "C", -3}}, {}}, 10);
  jobs_center += cxsom::jobs::pattern::make({"main", "D"}, {cxsom::jobs::Operation::Average, {{"main", "D", -1}, {"main", "D", -2}, {"main", "D", -3}}, {}}, 10);
  jobs_center.interaction_release();
  
  std::cout << "Done." << std::endl;

  for(auto& w : workers) w.join();
  return 0;
}