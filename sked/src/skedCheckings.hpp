#pragma once

#include <skedConcepts.hpp>
#include <skedQueue.hpp>

namespace sked {
  namespace checking {

    static_assert(sked::concepts::queue<sked::queue>);
    static_assert(sked::concepts::ack_queue<sked::ack::queue>);
    static_assert(sked::concepts::ack_queue<sked::double_buffered::queue>);
    static_assert(sked::concepts::ack_queue<sked::serial<sked::ack::queue>>);
    static_assert(sked::concepts::ack_queue<sked::serial<sked::double_buffered::queue>>);
    
  }
}
