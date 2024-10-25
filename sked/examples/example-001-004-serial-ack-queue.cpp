
#include <sked.hpp>
#include <vector>
#include <random>

#define NB_THREADS 10
#define DT .3 // small duration, such as the message is visible on the timeline.


#include "colormap.hpp"

int main(int argc, char* argv[]) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<unsigned int> job_duration(2, 5);
  
  // Any queue with job acknowledgment can be made serial, i.e. one
  // thread works at a time.
  sked::serial<sked::ack::queue> queue;
  
  sked::json::timeline timeline("timeline-001-004.tml");
  // Usage:
  // timeline(<thread-id>, <msg>, <duration>, <color>);
  
  colormap cmap;
  
  std::vector<std::thread> tasks;
  
  for(unsigned int i = 1; i <= NB_THREADS; ++i)
    tasks.emplace_back([i, &timeline, &queue, &cmap, &gen, &job_duration]() {
      timeline(i, "starts", job_duration(gen), cmap.preparer);
      timeline(i, "wait for job execution", DT, cmap.readyr);
      {
	auto in_job = sked::job_scope(queue);
	
	timeline(i, "start job", job_duration(gen), cmap.startr);
	timeline(i, "job done", DT, cmap.done);
      }
      
      timeline(i, "after work", 5, cmap.after);
      timeline(i, "finished", DT, cmap.finish);
    });

  timeline("sleeping", 3, cmap.wait);
  timeline("flushing now", DT, cmap.sync);
  while(queue.flush());
  timeline("All jobs done, joining now", DT, cmap.wait);
  for(auto& t : tasks) t.join();
  timeline("joined", DT, cmap.finish);
  
  std::cout << std::endl
	    << std::endl
	    << "You can use pysked-timeline-to-pdf to view the generated timeline-001-004.tml file." << std::endl
	    << std::endl;

  return 0;
}
