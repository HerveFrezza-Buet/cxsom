#include <sked.hpp>
#include <vector>
#include <string>

#define NB_THREADS 10
#define NB_ROUNDS  10
#define NB_JOBS     2

int main(int argc, char* argv[]) {
  sked::double_buffered::queue queue;
  sked::verbose::timer t;
  
  std::vector<std::thread> tasks;
  
  for(unsigned int i = 1; i <= NB_THREADS; ++i)
    tasks.emplace_back([i, &t, &queue]() {
      sked::verbose::message(t, i, "starts", i);
      for(int job = 1; job <= NB_JOBS; ++job)  {
	std::string job_id = std::to_string(job);
	sked::verbose::message(t, i, std::string("preparing job ") + job_id, 2);
	sked::verbose::message(t, i, std::string("ready for job ") + job_id, 0);
	{
	  auto in_job = sked::job_scope(queue);
	  sked::verbose::message(t, i, std::string("start job ") + job_id, NB_THREADS + 1 - i);
	  sked::verbose::message(t, i, std::string("job ") + job_id + " done",0); 
	}
      }
      sked::verbose::message(t, i, "after work", 5);
      sked::verbose::message(t, i, "finished",   0);
    });
  
  sked::verbose::message(t, "sleeping", 5);
  for(int round = 1; round <= NB_ROUNDS; ++round) {
    sked::verbose::message(t, std::string("Round ") + std::to_string(round), 0);
    queue.flush();
  }

  sked::verbose::message(t, "All rounds done, joining now", 0);
  for(auto& t : tasks) t.join();

  return 0;
}
