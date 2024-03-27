#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <thread>
#include <iterator>

#include <utility> // should be included by asio
#include <asio.hpp>

#include <skednet.hpp>


#define NB_THREADS 5
#define NB_ROUNDS 10
#define WRITER_PROBA .2

struct colormap {
  sked::json::rgb preparer {.5, .5, .7};
  sked::json::rgb readyr   {0., 0., 5.};
  sked::json::rgb startr   {.5, .5, 1.};
    
  sked::json::rgb preparew {.7, .5, .5};
  sked::json::rgb readyw   {.5, 0., 0.};
  sked::json::rgb startw   {1., .5, .5};
    
  sked::json::rgb count  {0., .7, 0.};
  sked::json::rgb sync   {.2, .2, .2};
  sked::json::rgb wait   {.5, .7, .5};
    
  sked::json::rgb after   {.8, .8, .3};
  sked::json::rgb done    {0., 0., 0};
};

void process(sked::json::timeline& timeline, unsigned int id, const std::string& hostname, const std::string& port) {
  asio::ip::tcp::iostream socket;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<unsigned int> job_duration(1, 3);
  std::uniform_real_distribution<double> toss(0, 1);
  sked::json::rgb write_color {.8, .2, .2};
  sked::json::rgb read_color  {.2, .8, .2};
  try {
    socket.exceptions(std::ios::failbit | std::ios::badbit | std::ios::eofbit);
    socket.connect(hostname, port);

    for(unsigned int i = 0; i < NB_ROUNDS; ++i)
      if(toss(gen) < WRITER_PROBA) {
	sked::net::scope::xrsw::write lock {socket, socket};
	timeline(id, "write", job_duration(gen), write_color);
      }
      else {
	sked::net::scope::xrsw::read lock {socket, socket};
	timeline(id, "read", job_duration(gen), read_color);
      }
  }
  catch(std::exception& e) {
    std::cerr << "Exception caught : " << e.what() << " --> " << typeid(e).name()<< std::endl;
  }
}

int main(int argc, char* argv[]) {

  if(argc != 3) {
    std::cout << "Usage: " << argv[0] << " <hostname> <port>" << std::endl
	      << "       (start skednet-xrsw server beforehand)." << std::endl;
    return  0;
  }

  std::string hostname {argv[1]};
  std::string port     {argv[2]};
  sked::json::timeline timeline("timeline-001-001.tml");
  sked::json::rgb wait_color  {.6, .4, .5};
  std::vector<std::thread> threads;

  unsigned int thread_id = 0;
  for(int i = 0; i < 2; ++i) {
    for(unsigned int t = 0; t < NB_THREADS; ++t)
      threads.emplace_back([&timeline, id=++thread_id, hostname, port](){process(timeline, id, hostname, port);});
    timeline("Waiting for next threads", 10, wait_color);
  }
  timeline("All threads are started.", 2, wait_color);
  for(auto& t: threads) t.join();

  
  std::cout << std::endl
	    << std::endl
	    << "You can use pysked-timeline-to-pdf to view the generated timeline-001-001.tml file." << std::endl
	    << std::endl;
  return 0;
  
}

