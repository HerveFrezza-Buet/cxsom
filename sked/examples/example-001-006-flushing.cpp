#include <sked.hpp>
#include <string>
#include <random>
#include <atomic>

#define NB_THREADS 20
#define DT .3 // small duration, such as the message is visible on the timeline.

#include "colormap.hpp"

int main(int argc, char* argv[]) {
  sked::double_buffered::queue queue;
  
  sked::json::timeline timeline("timeline-001-006.tml");
  // Usage:
  // timeline(<thread-id>, <msg>, <duration>, <color>);
  
  std::atomic<unsigned int> remaining {NB_THREADS};

  colormap cmap;

  for(unsigned int i = 1; i <= NB_THREADS; ++i)
    std::thread([i, &timeline, &queue, &remaining, &cmap]() {
      std::random_device rd;
      std::mt19937 gen(rd());
      auto wake_up_duration = std::uniform_real_distribution<double>(5, 10)(gen);
      auto job_duration     = std::uniform_real_distribution<double>(1, 5)(gen);
      
      timeline(i, "wake up",       wake_up_duration, cmap.wait);
      timeline(i, "ready to work", DT, cmap.readyr);
      {
	auto in_job = sked::job_scope(queue);
	timeline(i, "start job ", job_duration, cmap.startr);
	timeline(i, "job done",   DT, cmap.done); 
      }
      timeline(i, "after work", 5, cmap.after);
      timeline(i, "finished",   DT, cmap.finish);
      remaining--;
    }).detach();

  // The main thread runs the queue processing.
  while(remaining > 0) {
    timeline(std::to_string(remaining) + " active thread(s).", DT, cmap.endwait);
    while(queue.flush())
      timeline("tasks performed", DT, cmap.sync);
    timeline("all flushes done, waiting", 1, cmap.finish);
  }
  timeline("No remaing threads, we are done", 1, cmap.after);


  std::cout << std::endl
	    << std::endl
	    << "You can use pysked-timeline-to-pdf to view the generated timeline-001-006.tml file." << std::endl
	    << std::endl;
  return 0;
}

  


      
