#include <sked.hpp>
#include <vector>

#define NB_THREADS 10
#define DT .3 // small duration, such as the message is visible on the timeline.

#include "colormap.hpp"

// Basic sked::queues enables tasks to get locked on a barrier. They
// are all unlocked when we flush.

int main(int argc, char* argv[]) {
  sked::queue queue;
  
  sked::json::timeline timeline("timeline-001-001.tml");

  // Usage:
  // timeline(<thread-id>, <msg>, <duration>, <color>);
  
  colormap cmap;
  
  std::vector<std::thread> tasks;

  for(unsigned int i = 1; i <= NB_THREADS; ++i)
    tasks.emplace_back([i, &timeline, &queue, &cmap]() {
      timeline(i, "starts",   i, cmap.preparer);
      timeline(i, "go ahead", DT, cmap.readyr);
      queue.go_ahead(); // blocked until the main thread flushes.
      timeline(i, "passed", 1, cmap.after);
      timeline(i, "finished", DT, cmap.done);
    });

  timeline("sleeping", NB_THREADS + 3, cmap.wait); // We wait all secondary threads to each go_ahead.
  timeline("flushing now", DT, cmap.sync);
  queue.flush();

  timeline("joining now", DT, cmap.wait);
  for(auto& t : tasks) t.join();
  timeline("joined", DT, cmap.done);


  std::cout << std::endl
	    << std::endl
	    << "You can use pysked-timeline-to-pdf.py to view the generated timeline-001-001.tml file." << std::endl
	    << std::endl;

  
  return 0;
}
