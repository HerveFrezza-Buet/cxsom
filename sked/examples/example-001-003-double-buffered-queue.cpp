#include <sked.hpp>
#include <vector>
#include <string>

#define NB_THREADS 10
#define NB_JOBS     2

#include "colormap.hpp"

// Double-bufferred queues host two queues internally. Jobs register to a 'back' queue when they ask for execution. When flush occurs, the back-queu becomes the front queue, and jobs in that queue are executed. 
  // When jobs get flushed, is another job is required, it get stored
  // in another queue fir exectution in a incomming flushing stage.

int main(int argc, char* argv[]) {
  sked::double_buffered::queue queue;
  sked::json::timeline timeline("timeline-001-003.tml");
  
  colormap cmap;
  
  std::vector<std::thread> tasks;
  
  for(unsigned int i = 1; i <= NB_THREADS; ++i)
    tasks.emplace_back([i, &timeline, &queue, &cmap]() {
      timeline(i, "starts", i, cmap.wait);
      for(int job = 1; job <= NB_JOBS; ++job)  {
	std::string job_id = std::to_string(job);
	timeline(i, std::string("preparing job ") + job_id, 2, cmap.preparer);
	timeline(i, std::string("ready for job ") + job_id, 0, cmap.readyr);
	{
	  auto in_job = sked::job_scope(queue);
	  timeline(i, std::string("start job ") + job_id, NB_THREADS + 1 - i, cmap.startr);
	  timeline(i, std::string("job ") + job_id + " done", 0, cmap.done); 
	}
      }
      timeline(i, "after work", 5, cmap.after);
      timeline(i, "finished",   0, cmap.done);
    });
  
  timeline("sleeping", 5, cmap.wait);
  unsigned int round = 1;
  do 
    timeline(std::string("Round ") + std::to_string(round++), 0, cmap.sync);
  while(queue.flush());

  timeline("All rounds done, joining now", 0, cmap.wait);
  for(auto& t : tasks) t.join();
  timeline("joined", 0, cmap.done);

  std::cout << std::endl
	    << std::endl
	    << "You can use pysked-timeline-to-pdf to view the generated timeline-001-003.tml file." << std::endl
	    << std::endl;
  return 0;
}
