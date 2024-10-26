#include <sked.hpp>
#include <string>
#include <random>
#include <atomic>

#define NB_THREADS 10
#define DT .3 // small duration, such as the message is visible on the timeline.
#define NB_ROUNDS   3

// xrsw stands for multiple-reader, single writer. A xrsw queue
// handles two double-buffered queues, one for the readers, one for
// the writers. The one for the writer is serial, so that only one
// writer writes at a time.
//
// Flushing consists in flusing writers first, and thean readers.

#include "colormap.hpp"

int main(int argc, char* argv[]) {
  sked::xrsw::queue queue;
  sked::json::timeline timeline("timeline-002-001.tml");

  // Usage:
  // timeline(<thread-id>, <msg>, <duration>, <color>);
  
  std::atomic<unsigned int> remaining {NB_THREADS};

  colormap cmap;
  
  for(unsigned int i = 1; i <= NB_THREADS; ++i)
    std::thread([i, &timeline, &queue, &remaining, &cmap]() {
      std::random_device rd;
      std::mt19937 gen(rd());

      for(unsigned int round = 1; round <= NB_ROUNDS; ++round) {
	std::string r = std::string("round ") + std::to_string(round) + " : ";
	std::uniform_real_distribution<double> delay(1, 5);
	
	timeline(i, r + "preparing write", delay(gen), cmap.preparew);
	timeline(i, r + "ready to write", DT, cmap.readyw);
	{
	  auto lock = sked::xrsw::writer_scope(queue);
	  timeline(i, r + "start write", 1, cmap.startw);
	  timeline(i, r + "writing done", DT, cmap.done); 
	}
	timeline(i, r + "preparing read", delay(gen), cmap.preparer);
	timeline(i, r + "ready to read", DT, cmap.readyr);
	{
	  auto lock = sked::xrsw::reader_scope(queue);
	  timeline(i, r + "start read", 1, cmap.startr);
	  timeline(i, r + "reading done", DT, cmap.done); 
	}
      }
      timeline(i, "after work", 5, cmap.after);
      timeline(i, "finished",   DT, cmap.finish);
      remaining--;
    }).detach();

  // The main thread runs the queue processing.
  while(remaining > 0) {
    timeline(std::to_string(remaining) + " active thread(s).", DT, cmap.count);
    while(queue.flush())
      timeline("tasks performed", DT, cmap.sync);
    timeline("all flushes done, waiting", 1, cmap.wait);
  }

  std::cout << std::endl
	    << std::endl
	    << "You can use pysked-timeline-to-pdf to view the generated timeline-002-001.tml file." << std::endl
	    << std::endl;
													    
  return 0;
}
