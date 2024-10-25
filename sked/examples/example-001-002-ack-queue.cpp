#include <sked.hpp>
#include <vector>

#define NB_THREADS 10
#define DT .3 // small duration, such as the message is visible on the timeline.

#include "colormap.hpp"

// The sked::ack::queue is a queue that 'knows' if some tasks are
// still waiting for execution. The flush call returns a boolean,
// telling wether jobs have been started. Flush blocks until all
// current jobs are done.

// While flushing is in progress, new jobs can enter the queue, this
// may increase the current flush duration.

int main(int argc, char* argv[]) {
  sked::ack::queue queue;
  sked::json::timeline timeline("timeline-001-002.tml");
  
  // Usage:
  // timeline(<thread-id>, <msg>, <duration>, <color>);
  
  colormap cmap;
  
  std::vector<std::thread> tasks;
  
  for(unsigned int i = 1; i <= NB_THREADS; ++i)
    tasks.emplace_back([i, &timeline, &queue, &cmap]() {
      timeline(i, "starts", i, cmap.preparer);
      timeline(i, "wait for job execution", DT, cmap.readyr);
      {
	auto in_job = sked::job_scope(queue);
	// auto ack_info = queue.go_ahead(); 
	
	timeline(i, "start job", 3, cmap.startr);
	timeline(i, "job done", DT, cmap.done);
	
	// queue.done(ac_info);
      }
      
      timeline(i, "after work", 5, cmap.after);
      timeline(i, "finished", DT, cmap.finish);
    });

  timeline("sleeping", 3, cmap.wait);
  unsigned int nb_flushes = 1;
  timeline("start flushing", DT, cmap.endwait);
  while(queue.flush())
    timeline(std::string("flush #") + std::to_string(nb_flushes++) + ".", DT, cmap.sync);
  
  timeline("All jobs done, joining now", DT, cmap.wait);
  for(auto& t : tasks) t.join();
  timeline("joined", DT, cmap.finish);
  
  std::cout << std::endl
	    << std::endl
	    << "You can use pysked-timeline-to-pdf.py to view the generated timeline-001-002.tml file." << std::endl
	    << std::endl;

  return 0;
}
