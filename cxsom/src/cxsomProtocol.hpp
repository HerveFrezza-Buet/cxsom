#pragma once


namespace cxsom {
  namespace error {
    struct parse : public std::logic_error {
      using std::logic_error::logic_error;
    };
  }
}
