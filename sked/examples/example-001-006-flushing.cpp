#include <sked.hpp>
#include <string>
#include <random>
#include <atomic>

#define NB_THREADS 20

int main(int argc, char* argv[]) {
  sked::double_buffered::queue queue;
  sked::verbose::timer t;
  std::atomic<unsigned int> remaining {NB_THREADS};

  for(unsigned int i = 1; i <= NB_THREADS; ++i)
    std::thread([i, &t, &queue, &remaining]() {
      std::random_device rd;
      std::mt19937 gen(rd());
      auto wake_up_duration = std::uniform_int_distribution<unsigned int>(5, 10)(gen);
      auto job_duration     = std::uniform_int_distribution<unsigned int>(1, 5)(gen);
      
      sked::verbose::message  (t, i, "wake up",       wake_up_duration);
      sked::verbose::message  (t, i, "ready to work", 0);
      {
	auto in_job = sked::job_scope(queue);
	sked::verbose::message(t, i, "start job ", job_duration);
	sked::verbose::message(t, i, "job done",   0); 
      }
      sked::verbose::message  (t, i, "after work", 5);
      sked::verbose::message  (t, i, "finished",   0);
      remaining--;
    }).detach();

  // The main thread runs the queue processing.
  while(remaining > 0) {
    sked::verbose::message(t, std::to_string(remaining) + " active thread(s).", 0);
    while(queue.flush())
      sked::verbose::message(t, "tasks performed", 0);
    sked::verbose::message(t, "all flushes done, waiting", 1);
  }

  return 0;
}

  


      
