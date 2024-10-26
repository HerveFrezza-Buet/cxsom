#pragma once

#include <skedQueue.hpp>

namespace sked {
  namespace xrsw {
    /**
     * XRSW stands for multi-readers/serial-writers. A xrsw::queue
     * handles two double-buffered queues, one for the readers, one
     * for the writers. The one for the writer is serial, so that only
     * one writer writes at a time.
     */
    class queue {
    public:

      using readers_queue_type = sked::double_buffered::queue;
      using writers_queue_type = sked::serial<readers_queue_type>;
      
    private:
      readers_queue_type readers_queue;
      writers_queue_type writers_queue;

    public:
      
      queue() = default;
      queue(const queue&) = delete;
      queue(queue&&) = delete;
      queue& operator=(const queue&) = delete;
      queue& operator=(queue&&) = delete;

      auto reader_scope() {return sked::job_scope(readers_queue);}
      auto writer_scope() {return sked::job_scope(writers_queue);}
      
      bool flush() {
	auto has_writing_jobs = writers_queue.flush();
	auto has_reading_jobs = readers_queue.flush();
	return has_writing_jobs || has_reading_jobs;
      }
    };

    auto reader_scope(queue& owner) {return owner.reader_scope();}
    auto writer_scope(queue& owner) {return owner.writer_scope();}
  }
}
