#pragma once


#include <algorithm>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include <skedConcepts.hpp>

namespace sked {

  class queue {
  protected:
    
  std::condition_variable to_do;
  std::mutex              to_do_mutex;
    
  public:
    queue() = default;
    queue(const queue&) = delete;
    queue(queue&&) = delete;
    queue& operator=(const queue&) = delete;
    queue& operator=(queue&&) = delete;

    void go_ahead() {
      std::unique_lock<std::mutex> lock(to_do_mutex);
      to_do.wait(lock);
    }
    void flush() {to_do.notify_all();}
  };

  template<concepts::ack_queue QUEUE>
  class job_scope_t {
  private:
    QUEUE& owner;
    typename QUEUE::ack_info_type ack_info;
    
  public:
    job_scope_t(QUEUE& owner) : owner(owner), ack_info() {ack_info = owner.go_ahead();}
    ~job_scope_t() {owner.done(ack_info);}
    
    job_scope_t() = delete;
    job_scope_t(const job_scope_t&) = delete;
    job_scope_t(job_scope_t&&) = delete;
    job_scope_t& operator=(const job_scope_t&) = delete;
    job_scope_t& operator=(job_scope_t&&) = delete;
  };

  
  template<concepts::ack_queue QUEUE>
  auto job_scope(QUEUE& owner) {return job_scope_t<QUEUE>(owner);}


  template<concepts::ack_queue QUEUE>
  class serial : public QUEUE {
  private:
    std::mutex exec_mutex;
    
  public:
    using ack_info_type = typename QUEUE::ack_info_type;
    
    serial() = default;
    serial(const serial&) = delete;
    serial(serial&&) = delete;
    serial& operator=(const serial&) = delete;
    serial& operator=(serial&&) = delete;
    
    ack_info_type go_ahead() {
      auto res = this->QUEUE::go_ahead();
      exec_mutex.lock();
      return res;
    }
    
    void done(ack_info_type ai) {
      exec_mutex.unlock();
      this->QUEUE::done(ai);
    }
  };
  
  namespace ack {

    struct no_info {};

    class queue : public sked::queue {
    private:
      std::condition_variable   all_done;
      std::mutex                all_done_mutex;
      std::atomic<unsigned int> size = 0;
      
      bool skip_when_empty() {
	bool res = false;
	if(size > 0) {
	  res = true;
	  std::unique_lock<std::mutex> lock(all_done_mutex);
	  all_done.wait(lock);
	}
	return res;
      }
      
    public:
      using ack_info_type = no_info;
      
      queue() = default;
      queue(const queue&) = delete;
      queue(queue&&) = delete;
      queue& operator=(const queue&) = delete;
      queue& operator=(queue&&) = delete;
      
      ack_info_type go_ahead() {
	std::unique_lock<std::mutex> lock(to_do_mutex);
	size++;
	to_do.wait(lock);
	return {};
      }

      /**
       * @return true if there were actual pending jobs.
       */
      bool flush() {
	this->sked::queue::flush();
	return skip_when_empty();
      }

      void done(ack_info_type) {
	if(--size == 0) all_done.notify_all();
      }
    };
  }

  namespace double_buffered {
    class queue {
    private:
      std::mutex swap_mutex;

      sked::ack::queue queue_a;
      sked::ack::queue queue_b;
      sked::ack::queue* front;
      sked::ack::queue* back;
      
    public:
      using ack_info_type = sked::ack::queue*;
	
      queue() : swap_mutex(), queue_a(), queue_b(), front(), back() {
	front = &queue_a;
	back  = &queue_b;
      }
      
      queue(const queue&) = delete;
      queue(queue&&) = delete;
      queue& operator=(const queue&) = delete;
      queue& operator=(queue&&) = delete;
      
      ack_info_type go_ahead() {
	sked::ack::queue* q_ptr;
	{
	  std::unique_lock<std::mutex> lock(swap_mutex);
	  q_ptr = back;
	}
	q_ptr->go_ahead();
	return q_ptr;
      }

      bool flush() {
	sked::ack::queue* q_ptr;
	{
	  std::unique_lock<std::mutex> lock(swap_mutex);
	  q_ptr = back;
	  std::swap(front, back);
	}
	return q_ptr->flush();
      }

      void done(ack_info_type q_ptr) {
	q_ptr->done({});
      }
    };
  }

}
