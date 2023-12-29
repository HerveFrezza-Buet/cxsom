#include <sked.hpp>
#include <vector>

#define NB_THREADS 10

#include "colormap.hpp"

// The sked::ack::queue is a queue that 'knows' if some tasks are
// still in progress. The flush call returns a boolean.

int main(int argc, char* argv[]) {
  sked::ack::queue queue; // Tasks in this queue acknowledge when they are done.
  sked::json::timeline timeline("timeline-001-002.tml");
  
  colormap cmap;
  
  std::vector<std::thread> tasks;
  
  for(unsigned int i = 1; i <= NB_THREADS; ++i)
    tasks.emplace_back([i, &timeline, &queue, &cmap]() {
      timeline(i, "starts", i, cmap.preparer);
      timeline(i, "wait for job execution", 0, cmap.readyr);
      {
	auto in_job = sked::job_scope(queue);
	// auto ack_info = queue.go_ahead(); 
	
	timeline(i, "start job", NB_THREADS + 1 - i, cmap.startr);
	timeline(i, "job done", 0, cmap.done);
	
	// queue.done(ac_info);
      }
      
      timeline(i, "after work", 5, cmap.after);
      timeline(i, "finished", 0, cmap.done);
    });

  timeline("sleeping", NB_THREADS + 3, cmap.wait);
  timeline("flushing now", 0, cmap.sync);
  if(queue.flush()) // true if pending jobs are remaining
    std::cout << "No jobs should be still pending... this message will never be printed." << std::endl;
  timeline("All jobs done, joining now", 0, cmap.wait);
  for(auto& t : tasks) t.join();
  timeline("joined", 0, cmap.done);
  
  std::cout << std::endl
	    << std::endl
	    << "You can use pysked-timeline-to-pdf.py to view the generated timeline-001-002.tml file." << std::endl
	    << std::endl;

  return 0;
}
