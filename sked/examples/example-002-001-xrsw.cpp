#include <sked.hpp>
#include <string>
#include <random>
#include <atomic>

#define NB_THREADS 5


int main(int argc, char* argv[]) {
  sked::xrsw::queue queue;
  sked::json::timeline timeline("timeline-002-001.json");
  std::atomic<unsigned int> remaining {NB_THREADS};

  for(unsigned int i = 1; i <= NB_THREADS; ++i)
    std::thread([i, &timeline, &queue, &remaining]() {
      std::random_device rd;
      std::mt19937 gen(rd());

      for(unsigned int round = 1; round <=3; ++round) {
	std::string r = std::string("round ") + std::to_string(round) + " : ";
	std::uniform_int_distribution<unsigned int> delay(0, 3);
	
	timeline(i, r + "preparing write", delay(gen));
	timeline(i, r + "ready to write", 0);
	{
	  auto lock = sked::xrsw::writer_scope(queue);
	  timeline(i, r + "start write", 1);
	  timeline(i, r + "writing done", 0); 
	}
	timeline(i, r + "preparing read", delay(gen));
	timeline(i, r + "ready to read", 0);
	{
	  auto lock = sked::xrsw::reader_scope(queue);
	  timeline(i, r + "start read", 1);
	  timeline(i, r + "reading done", 0); 
	}
      }
      timeline(i, "after work", 5);
      timeline(i, "finished",   0);
      remaining--;
    }).detach();

  // The main thread runs the queue processing.
  while(remaining > 0) {
    timeline(std::to_string(remaining) + " active thread(s).", 0);
    while(queue.flush())
      timeline("tasks performed", 0);
    timeline("all flushes done, waiting", 1);
  }

  return 0;
}
