#pragma once

/*

  This file gathers the definition that will be necessary for
  extending the set of available operations in cxsom to new
  ones. Here, the "foo" namespace will gather some specific
  extention. Indeed, we will consider a single supplementary
  operation, called "midbound". The midbound function is such as :

  midbound(a1, ... an) = (min(a1, ... an) + max(a1, ... an)) / 2

  The ai must all be of the same kind, that can any of the cxsom
  numerical data types.

*/

#include <iostream>

// Everything you need comes from that header.
#include <cxsom-server.hpp>

// We will name "foo-midbound" our midbound operator.
#define foo_MIDBOUND_NAME "foo-midbound"

namespace foo {


  // Let us start with the easiest part, the type checker. Indeed,
  // when the operation is involed in a rule and sent to the
  // cxsom-processor (the server), the type of the arguments are
  // checked (at execution time). We have to provide this function for
  // our midbound operator.

  // This function returns nothing, but raises an exception in case of bad typing.
  inline void check_types_midbound(cxsom::type::ref res, const std::vector<cxsom::type::ref>& args) {
    // Here, all arguments must be of the same type, which is also the
    // type of the result.  Let us suppose that we want at least 3
    // arguments to the operator. Let us also suppose (but it is not really
    // relevant for this operator) that we forbid the use of midbound
    // for maps containing arrays.
    
    std::ostringstream ostr;
    bool dummy = false;
   
    if(args.size >= 3) {
      // Let us check that the result is not a map of array.
      if(res->is_MapOfArray() == 0) {
	// You can find many other testing methods for
	// cxsom::type::ref, see the doc of the cxsom::type::Base
	// class.

	// For specific types, you can donwcast to make further
	// checks. Here are examples which are given for the sake of
	// illustration, but not used for our actual type checking...
	if(dummy) { // ... so this is never executed since dummy == false.
	  if(res->is_Array()) {
	    auto res_array = std::static_pointer_cast<cxsom::type::Array>(res);
	    std::cout << res_array->size // The dimension of the array.
		      << std::endl;
	  }
	  if(res->is_Map()) {
	    auto res_map = std::static_pointer_cast<cxsom::type::Map>(res);
	    std::cout << res_map->side // The map side (side*side for a 2D map)
		      << res_map->size // is side for 1D maps, side*size for a 2D map.
		      << res_map->content_size // The number of doubles for one of the elements.
		      << res_map->nb_of_doubles // The total number of doubles required to store the map elements.
		      << res_map->content_type // The string describing the content.
		      << std::endl;
	  }
	}

	bool all_args_have_the_result_type = true;
	auto it = args.begin();
	while(all_args_have_the_result_type && it != args.end())
	  all_args_have_the_result_type = (it->name() == res->name());
	if(all_args_have_the_result_type)
	  return;
	ostr << "Checking types for midbound : Some arguments are not of result type (" << res->name() << ").";
      }
      else // The result has a "MapXD<Array=...>=..." type.
	ostr << "Checking types for midbound : Maps of array are not allowed.";
    }
    else // nb_args < 3
      ostr << "Checking types for midbound : more than 3 arguments are required.";
    
    throw error::bad_typing(ostr.str());
  }

  // The previous time checking function will have to be notified to
  // some type checker. Let us write a function for registering all the
  // type checkings in foo (ok... only one here).
  void fill(cxsom::jobs::TypeChecker& type_checker) {
    type_checker += {foo_MIDBOUND_NAME, check_types_midbound};
  }

  // Now comes the actual computation. We need to define a so called
  // "update", inheriting from the cxsom::job::Base class. The latter
  // already inherits from cxsom::update::Base class. We will have to
  // override virtual methods defined there.

  class MidBound : public cxsom::jobs::Base {
  private:

  public:
    MidBound(data::Center& center,
	     const update::arg& res,
	     const std::vector<update::arg>& args,
	     const std::map<std::string, std::string>& params)
      : jobs::Base(center, foo_MIDBOUND_NAME, res, args) {
      // The last argument is the parameters (the key is the parameter
      // name, the value the parameter value). It can be used in the
      // constructor to apply settings defined by the user when s/he
      // writes a rule. We ignore it here.
    }

  protected:

    // This is where ou inherit callbacks for computation. See the
    // cxsom::update::Base class.

    virtual void on_computation_start() override {}
      
    virtual void on_read_out_arg(const symbol::Instance&, unsigned int, const data::Base&) override {}

    virtual void on_read_out_arg_aborted() override {}
    
    virtual void on_read_in_arg(const symbol::Instance&, unsigned int, const data::Base&) override {}
      
    virtual bool on_write_result(data::Base&) override {}
    
  };
}
