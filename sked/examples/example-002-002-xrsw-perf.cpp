#include <sked.hpp>
#include <string>
#include <random>
#include <atomic>
#include <iostream>
#include <cstddef>
#include <cstdlib>
#include <chrono>

#include "colormap.hpp"

#define READ_MS    100
#define WRITE_MS   100
#define WAIT_MS     20
#define WAKEUP_MS 1000

#define PRINT_PERIOD 20
#define PRINT_PERIOD_ (PRINT_PERIOD-1)

int main(int argc, char* argv[]) {
  sked::xrsw::queue queue;
  sked::json::timeline timeline("timeline-002-002.tml");


  if(argc != 3) {
    std::cout << "Usage: " << argv[0] << " <nb-threads> <nb-iters>" << std::endl;
    return 1;
  }
  
  std::size_t nb_threads {std::stoul(argv[1])};
  std::size_t nb_iters   {std::stoul(argv[2])};
  std::atomic<std::size_t> remaining {nb_threads};
  colormap cmap;

  for(unsigned int i = 1; i <= nb_threads; ++i)
    std::thread([i, nb_iters, &timeline, &queue, &remaining, &cmap]() {
      std::this_thread::sleep_for(std::chrono::milliseconds((unsigned int)(WAKEUP_MS)));
      auto read_duration = std::chrono::milliseconds((unsigned int)(READ_MS));
      for(unsigned int round = 1; round <= nb_iters; ) {
	{
	  auto lock = sked::xrsw::reader_scope(queue);
	  timeline(i, "reading", READ_MS*1.e-3, cmap.readyr);
	  ++round;
	}
	{
	  auto lock = sked::xrsw::reader_scope(queue);
	  timeline(i, "reading", READ_MS*1.e-3, cmap.startr);
	  ++round;
	}
	for(unsigned int r = 1; r < PRINT_PERIOD_ && round <= nb_iters; ++r, ++round) {
	  auto lock = sked::xrsw::reader_scope(queue);      
	  std::this_thread::sleep_for(read_duration);
	}
      }
      remaining--;
    }).detach();

  std::thread writer {[nb_iters, &timeline, &queue, &cmap]() {
      std::this_thread::sleep_for(std::chrono::milliseconds((unsigned int)(WAKEUP_MS)));
      auto write_duration = std::chrono::milliseconds((unsigned int)(WRITE_MS));
      for(unsigned int round = 1; round <= nb_iters; ) {
	{
	  auto lock = sked::xrsw::writer_scope(queue);
	  timeline(0, "writing", WRITE_MS*1.e-3, cmap.readyw);  
	  ++round;
	}
	{
	  auto lock = sked::xrsw::writer_scope(queue);
	  timeline(0, "writing", WRITE_MS*1.e-3, cmap.startw);  
	  ++round;
	}
	for(unsigned int r = 1; r < PRINT_PERIOD_ && round <= nb_iters; ++r, ++round) {
	  auto lock = sked::xrsw::writer_scope(queue);      
	  std::this_thread::sleep_for(write_duration);
	}
      }
  }};

  sked::verbose::timer t {};
  
  while(remaining > 0) {
    while(queue.flush());
    std::this_thread::sleep_for(std::chrono::milliseconds((unsigned int)(WAIT_MS)));
  }

  writer.join();
  timeline("all flushes done", .1, cmap.sync);

  double ideal = 1e-3*(WAKEUP_MS + nb_iters * (READ_MS+WRITE_MS));
		
  std::cout << std::endl
	    << std::endl
	    << std::endl
	    << t() << "s, ideally " << ideal << "s." << std::endl
	    << std::endl
	    << "You can use pysked-timeline-to-pdf to view the generated timeline-002-001.tml file." << std::endl
	    << std::endl;
  
  return 0;
}
