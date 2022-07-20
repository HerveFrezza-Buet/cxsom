#pragma once

#include <vector>

// Everything you need comes from that header.
#include <cxsom-rules.hpp>


#define foo_MIDBOUND_NAME "foo-midbound"

// We have to define here some functions that can be invoked by the user when s/he defines computation rules (at client side)
namespace foo {
  namespace fx {
    // Here, we provide 3 function calls for the midbound operator.

    // "X" << foo::fx::mid_bound("A", "B");
    inline cxsom::rules::update mid_bound(cxsom::rules::kwd::data arg1,
					  cxsom::rules::kwd::data arg2) {
      // The ctx variable (context) is available from previously
      // included headers.  Passing args like (*ctx)(arg) enables a
      // declaration of the argument in the cxsom context.
      return {foo_MIDBOUND_NAME, {(*ctx)(arg1), (*ctx)(arg2)}};
    }
    
    // "X" << foo::fx::mid_bound("A", "B", "C");
    inline cxsom::rules::update mid_bound(cxsom::rules::kwd::data arg1,
					  cxsom::rules::kwd::data arg2,
					  cxsom::rules::kwd::data arg3) {
      return {foo_MIDBOUND_NAME, {(*ctx)(arg1), (*ctx)(arg2), (*ctx)(arg3)}};
    }
    
    // "X" << foo::fx::mid_bound({kwd::data("A"), "B", "C", "D", "E"});
    inline cxsom::rules::update mid_bound(const std::vector<cxsom::rules::kwd::data>& args) {
	std::vector<cxsom::rules::kwd::data> declared_args;
	for(auto& arg : args)
	  declared_args.push_back((*ctx)(arg));
	return {foo_MIDBOUND_NAME, declared_args};
    }
  }
}
