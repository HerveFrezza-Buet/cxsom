#pragma once

#include <concepts>

namespace sked {
  namespace concepts {
    
    template<typename QUEUE>
    concept queue =
      std::default_initializable<QUEUE>
      && requires(QUEUE p) {
      p.go_ahead();
      p.flush();
    };

    template<typename QUEUE>
    concept ack_queue =
      queue<QUEUE>
      && requires {
      typename QUEUE::ack_info_type;
    }
      && requires(QUEUE p, const typename QUEUE::ack_info_type cait) {
      {p.go_ahead()} -> std::same_as<typename QUEUE::ack_info_type>;
      p.done(cait);
      {p.flush()} -> std::same_as<bool>;
    };
  }
}
