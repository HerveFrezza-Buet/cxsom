#include <sked.hpp>
#include <string>
#include <random>
#include <atomic>

#define NB_THREADS 10
#define NB_ROUNDS   3


int main(int argc, char* argv[]) {
  sked::xrsw::queue queue;
  sked::json::timeline timeline("timeline-002-001.tml");
  std::atomic<unsigned int> remaining {NB_THREADS};

  struct colormap {
    sked::json::rgb preparer {.5, .5, .7};
    sked::json::rgb readyr   {0., 0., 1.};
    sked::json::rgb startr   {.2, .2, 1.};
    
    sked::json::rgb preparew {7., .5, .5};
    sked::json::rgb readyw   {1., 0., 0.};
    sked::json::rgb startw   {1., .2, .2};
    
    sked::json::rgb count  {0., .7, 0.};
    sked::json::rgb sync   {0., .0, 0.};
    sked::json::rgb wait   {.5, .7, .5};
    
    sked::json::rgb after   {.8, 8., .3};
    sked::json::rgb done    {0., 0., 0};
  } cmap;

  for(unsigned int i = 1; i <= NB_THREADS; ++i)
    std::thread([i, &timeline, &queue, &remaining, &cmap]() {
      std::random_device rd;
      std::mt19937 gen(rd());

      for(unsigned int round = 1; round <= NB_ROUNDS; ++round) {
	std::string r = std::string("round ") + std::to_string(round) + " : ";
	std::uniform_int_distribution<unsigned int> delay(1, 5);
	
	timeline(i, r + "preparing write", delay(gen), cmap.preparew);
	timeline(i, r + "ready to write", 0, cmap.readyw);
	{
	  auto lock = sked::xrsw::writer_scope(queue);
	  timeline(i, r + "start write", 1, cmap.startw);
	  timeline(i, r + "writing done", 0, cmap.done); 
	}
	timeline(i, r + "preparing read", delay(gen), cmap.preparer);
	timeline(i, r + "ready to read", 0, cmap.readyr);
	{
	  auto lock = sked::xrsw::reader_scope(queue);
	  timeline(i, r + "start read", 1, cmap.startr);
	  timeline(i, r + "reading done", 0, cmap.done); 
	}
      }
      timeline(i, "after work", 5, cmap.after);
      timeline(i, "finished",   0, cmap.done);
      remaining--;
    }).detach();

  // The main thread runs the queue processing.
  while(remaining > 0) {
    timeline(std::to_string(remaining) + " active thread(s).", 0, cmap.count);
    while(queue.flush())
      timeline("tasks performed", 0, cmap.sync);
    timeline("all flushes done, waiting", 1, cmap.wait);
  }

  std::cout << std::endl
	    << std::endl
	    << "You can use pysked-timeline-to-pdf to view the generated timeline-002-001.tml file." << std::endl
	    << std::endl;
													    
  return 0;
}
