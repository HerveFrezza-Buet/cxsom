#include <sked.hpp>
#include <vector>

#define NB_THREADS 10

int main(int argc, char* argv[]) {
  sked::ack::queue queue; // Thasks in this queue acknowledge when they are done.
  sked::verbose::timer t;
  
  std::vector<std::thread> tasks;
  
  for(unsigned int i = 1; i <= NB_THREADS; ++i)
    tasks.emplace_back([i, &t, &queue]() {
      sked::verbose::message(t, i, "starts",   i);
      sked::verbose::message(t, i, "wait for job execution", 0);
      {
	auto in_job = sked::job_scope(queue);
	// auto ack_info = queue.go_ahead(); 
	
	sked::verbose::message(t, i, "start job", NB_THREADS + 1 - i);
	sked::verbose::message(t, i, "job done",                   0);
	
	// queue.done(ac_info);
      }
      
      sked::verbose::message(t, i, "after work", 5);
      sked::verbose::message(t, i, "finished",   0);
    });

  sked::verbose::message(t, "sleeping", NB_THREADS+3);
  sked::verbose::message(t, "flushing now", 0);
  queue.flush();
  sked::verbose::message(t, "All jobs done, joining now", 0);
  for(auto& t : tasks) t.join();

  return 0;
}
