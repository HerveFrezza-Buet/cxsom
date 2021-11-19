#pragma once


namespace cxsom {
  namespace builder {

    struct Dot;
    using ref_dot = std::shared_ptr<Dot>;
    
    struct Variable;
    using ref_variable = std::shared_ptr<Variable>;

    struct Map;
    using ref_map = std::shared_ptr<Map>;

    struct Architecture;
    using ref_architecture = std::shared_ptr<Architecture>;
  }
}
