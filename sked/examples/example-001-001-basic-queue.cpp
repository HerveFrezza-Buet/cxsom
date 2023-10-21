#include <sked.hpp>
#include <vector>

#define NB_THREADS 10

int main(int argc, char* argv[]) {
  sked::queue queue;
  sked::verbose::timer t;
  
  std::vector<std::thread> tasks;

  for(unsigned int i = 1; i <= NB_THREADS; ++i)
    tasks.emplace_back([i, &t, &queue]() {
      sked::verbose::message(t, i, "starts",   i);
      sked::verbose::message(t, i, "go ahead", 0);
      queue.go_ahead(); // blocking
      sked::verbose::message(t, i, "passed", NB_THREADS + 1 - i);
      sked::verbose::message(t, i, "finished", 0);
    });

  sked::verbose::message(t, "sleeping", NB_THREADS+3);
  sked::verbose::message(t, "flushing now", 0);
  queue.flush();

  sked::verbose::message(t, "joining now", 0);
  for(auto& t : tasks) t.join();

  return 0;
}
