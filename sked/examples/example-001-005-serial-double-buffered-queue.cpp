#include <sked.hpp>
#include <vector>
#include <string>
#include <random>

#define NB_THREADS 10
#define DT .3 // small duration, such as the message is visible on the timeline.
#define NB_ROUNDS  10
#define NB_JOBS     2

#include "colormap.hpp"

// We can make a double-buffered queue serial.

int main(int argc, char* argv[]) {
  
  sked::serial<sked::double_buffered::queue> queue;
  sked::json::timeline timeline("timeline-001-005.tml");
  // Usage:
  // timeline(<thread-id>, <msg>, <duration>, <color>);
  
  colormap cmap;
  
  std::vector<std::thread> tasks;
  
  for(unsigned int i = 1; i <= NB_THREADS; ++i)
    tasks.emplace_back([i, &timeline, &queue, &cmap]() {
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_real_distribution<double> job_duration(3, 7);
  
      timeline(i, "starts", job_duration(gen), cmap.wait);
      for(int job = 1; job <= NB_JOBS; ++job)  {
	std::string job_id = std::to_string(job);
	timeline(i, std::string("preparing job ") + job_id, 2, cmap.preparer);
	timeline(i, std::string("ready for job ") + job_id, DT, cmap.readyr);
	{
	  auto in_job = sked::job_scope(queue);
	  timeline(i, std::string("start job ") + job_id, job_duration(gen), cmap.startr);
	  timeline(i, std::string("job ") + job_id + " done", DT, cmap.done); 
	}
      }
      timeline(i, "after work", 5, cmap.after);
      timeline(i, "finished",   DT, cmap.finish);
    });

  // We scan every 2 seconds if some job is to be executed.
  do {
    timeline("sleeping",  2, cmap.wait);
    timeline("waking up", DT, cmap.sync);
  } while(!queue.flush());
    
  unsigned int round = 1;
  do 
    timeline(std::string("Round ") + std::to_string(round++) + " done.", DT, cmap.sync);
  while(queue.flush());

  timeline("All rounds done, joining now", DT, cmap.after);
  for(auto& t : tasks) t.join();
  timeline("joined", DT, cmap.finish);
  

  std::cout << std::endl
	    << std::endl
	    << "You can use pysked-timeline-to-pdf to view the generated timeline-001-005.tml file." << std::endl
	    << std::endl;
  return 0;
}
