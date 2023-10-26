#include <sked.hpp>
#include <vector>

#define NB_THREADS 10

#include "colormap.hpp"

int main(int argc, char* argv[]) {
  sked::queue queue;
  sked::json::timeline timeline("timeline-001-001.tml");
  
  colormap cmap;
  
  std::vector<std::thread> tasks;

  for(unsigned int i = 1; i <= NB_THREADS; ++i)
    tasks.emplace_back([i, &timeline, &queue, &cmap]() {
      timeline(i, "starts",   i, cmap.preparer);
      timeline(i, "go ahead", 0, cmap.readyr);
      queue.go_ahead(); // blocking
      timeline(i, "passed", NB_THREADS + 1 - i, cmap.after);
      timeline(i, "finished", 0, cmap.done);
    });

  timeline("sleeping", NB_THREADS + 3, cmap.wait);
  timeline("flushing now", 0, cmap.sync);
  queue.flush();

  timeline("joining now", 0, cmap.wait);
  for(auto& t : tasks) t.join();
  timeline("joined", 0, cmap.done);


  std::cout << std::endl
	    << std::endl
	    << "You can use pysked-timeline-to-pdf to view the generated timeline-001-001.tml file." << std::endl
	    << std::endl;

  
  return 0;
}
