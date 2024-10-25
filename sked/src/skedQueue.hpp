#pragma once


#include <algorithm>
#include <mutex>
#include <condition_variable>
#include <atomic>


#include <skedConcepts.hpp>

namespace sked {

  /**
   * @short Basic wait-go queue.
   * A "queue" is an object shared by a main thread and many secondary threads. Secondary thread can call "q.go_ahead()" on the queue, when the ask for continuing execution. The main thread can call "q.flush()" to enable all waiting secondary threads to pass their "go_ahead" instruction.
   */
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

    /*
     * @short This ask for proceeding further.
     */
    void go_ahead() {
      std::unique_lock<std::mutex> lock(to_do_mutex);
      to_do.wait(lock);
    }
    void flush() {
      std::lock_guard<std::mutex> lock(to_do_mutex);
      to_do.notify_all();
    }
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


  /**
   * @short Wraps a queue so that secondary threads pass go_ahead one after the other.
   */
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

    /**
     * @short These queues acknowledge when they finish their job.
     * When a secondary thread passes the "q.go_ahead()" barrier, it performs a job and then notifies that it os done, by calling "q.done()". The go_ahead call returns an information that has to be passed to done, i.e. "info = q.go_ahead(); ....; q.done(info)". In the main thread, "q.flush()" waits for all running secondary threads to have acknowledge by "done". Some othe secondary threads my ask for "go_ahead" while flushing is in progress. The "q.flush()" call returns false when it triggers no secondary thread work.
     */
    class queue : public sked::queue {
    private:
      std::condition_variable   check_activity;
      std::mutex                check_activity_mutex;
      std::atomic<unsigned int> size = 0;
      
      
    public:
      using ack_info_type = no_info;
      
      queue() = default;
      queue(const queue&) = delete;
      queue(queue&&) = delete;
      queue& operator=(const queue&) = delete;
      queue& operator=(queue&&) = delete;
      
      ack_info_type go_ahead() {
	{
	  std::lock_guard<std::mutex> lock(check_activity_mutex);
	  check_activity.notify_one(); // There is only only the main thread pending.
	}
	{
	  std::unique_lock<std::mutex> lock(to_do_mutex);
	  size++;
	  to_do.wait(lock);
	}
	return {};
      }

      /**
       * @return true if there were actual pending jobs.
       */
      bool flush() {
	bool res = false;
	
	while(size > 0) {
	  res = true;
	  this->sked::queue::flush();
	  std::unique_lock<std::mutex> lock(check_activity_mutex);
	  check_activity.wait(lock);
	}
	return res;
      }

      void done(ack_info_type) {
	std::lock_guard<std::mutex> lock(check_activity_mutex);
	--size;
	check_activity.notify_one(); // There is only only the main thread pending.
      }
    };
  }

  namespace double_buffered {
    /**
     * @short This hosts a front and back queue internally.
     */
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
