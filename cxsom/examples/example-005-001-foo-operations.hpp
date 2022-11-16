#pragma once

/*

  This file gathers the definition that will be necessary for
  extending the set of available operations in cxsom to new
  ones. Here, the "foo" namespace will gather some specific
  extentions. Indeed, we will consider a single supplementary
  operation, called "midbound". The midbound function is such as :

  midbound(a1, ... an) = (min(a1, ... an) + max(a1, ... an)) / 2

  The ai must all be of the same kind, that can be any of the cxsom
  numerical data types... except maps of array.

*/

#include <iostream>
#include <algorithm>
#include <tuple>
#include <iterator>

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
    // type of the result.  Let us suppose that we want at least 2
    // arguments to the operator. Let us also suppose (but it is not really
    // relevant for this operator) that we forbid the use of midbound
    // for maps containing arrays.
    
    std::ostringstream ostr;
    bool dummy = false;
   
    if(args.size() >= 2) {
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
	    auto res_array = std::static_pointer_cast<const cxsom::type::Array>(res);
	    std::cout << res_array->size // The dimension of the array.
		      << std::endl;
	  }
	  if(res->is_Map()) {
	    auto res_map = std::static_pointer_cast<const cxsom::type::Map>(res);
	    std::cout << res_map->side // The map side (side*side for a 2D map)
		      << res_map->size // is side for 1D maps, side*side for a 2D map.
		      << res_map->content_size // The number of doubles for one of the elements.
		      << res_map->nb_of_doubles // The total number of doubles required to store the map elements.
		      << res_map->content_type // The string describing the content.
		      << std::endl;
	  }
	}

	bool all_args_have_the_result_type = true;
	for(auto it = args.begin(); all_args_have_the_result_type && it != args.end(); ++it)
	  all_args_have_the_result_type = ((*it)->name() == res->name());
	if(all_args_have_the_result_type)
	  return;
	ostr << "Checking types for midbound : Some arguments are not of result type (" << res->name() << ").";
      }
      else // The result has a "MapXD<Array=...>=..." type.
	ostr << "Checking types for midbound : Maps of array are not allowed.";
    }
    else // nb_args < 2
      ostr << "Checking types for midbound : more than 1 arguments are required.";
    
    throw cxsom::error::bad_typing(ostr.str());
  }

  // The previous time checking function will have to be notified to
  // some type checker. Let us write a function for registering all the
  // type checkings in foo (ok... only one here).
  void fill(cxsom::jobs::TypeChecker& type_checker) {
    type_checker += {foo_MIDBOUND_NAME, check_types_midbound}; // The type checking for our single operator.
  }

  // Now comes the actual computation. We need to define a so called
  // "update", inheriting from the cxsom::job::Base class. The latter
  // already inherits from cxsom::update::Base class. We will have to
  // override virtual methods defined there.

  class MidBound : public cxsom::jobs::Base {
  private:

    // The minimal change to be detected
    double epsilon = 0;
    
    // This is for the min/max of out-arguments (once it is computed, they will not change during relaxation).
    std::vector<double> out_mins;
    std::vector<double> out_maxs;
    bool out_mins_maxs_meaningful = false;
    bool are_there_out_args       = false;

    // This is for the min/max of in-arguments (this needs to be reconsidered when relaxation occurs)
    std::vector<double> in_mins;
    std::vector<double> in_maxs;
    bool in_mins_maxs_meaningful = false;

    // This is the type of everything (result and arguments). The fact
    // that all stuff have the same type has been checked by the type
    // checker, so we can trust this.
    cxsom::type::ref type = nullptr;


  public:
    MidBound(cxsom::data::Center& center,
	     const cxsom::update::arg& res,
	     const std::vector<cxsom::update::arg>& args,
	     const std::map<std::string, std::string>& params
	     /* std::mt19937::result_type seed */)
      // No seed needed, our computation is deterministic. Provide the seed argument
      // otherwise and use it for random processing.
      : cxsom::jobs::Base(center, res, foo_MIDBOUND_NAME, args),
	out_mins(), out_maxs(),
	in_mins(), in_maxs() {

      // We set epsilon if the pararametrs require to do so.
      if(auto it = params.find("epsilon"); it != params.end()) epsilon = std::stod(it->second);

      // We get the type
      type = std::get<1>(res);

      // Then, we resize the data holders accordingly.
      std::size_t size;
      if(type->is_Array())      size = std::static_pointer_cast<const cxsom::type::Array>(type)->size;
      else if(type->is_Map())   size = std::static_pointer_cast<const cxsom::type::Map>(type)->nb_of_doubles;
      else if(type->is_Pos2D()) size = 2;
      else                      size = 1;
      in_mins.resize(size);
      in_maxs.resize(size);
      out_mins.resize(size);
      out_maxs.resize(size);
    }

  private:

    // This is a tool function for getting pointers to the
    // [first, last) interval of the data memory chunk.
    std::tuple<double*, double*> get_data_range(cxsom::data::Base& arg_data) {
      double* data_begin = nullptr;
      double* data_end   = nullptr;

      // The data::Base class has a first_byte method that would have
      // avoided, the following if statements. Nevertheless, using it
      // would imply raw C-like casts.
      
      if(type->is_Scalar()) {
	data_begin = &(static_cast<cxsom::data::Scalar&>(arg_data).value);
	data_end = data_begin + 1;
      }
      else if(type->is_Pos1D()) {
	data_begin = &(static_cast<cxsom::data::d1::Pos&>(arg_data).x);
	data_end = data_begin + 1;
      }
      else if(type->is_Pos2D()) {
	data_begin = std::data(static_cast<cxsom::data::d2::Pos&>(arg_data).xy);
	data_end   = data_begin + 2;
      }
      else if(type->is_Array()) {
	data_begin = std::data(static_cast<cxsom::data::Array&>(arg_data).content);
	data_end   = data_begin + static_cast<cxsom::data::Array&>(arg_data).content.size();
      }
      else if(type->is_Map()) {
	data_begin = std::data(static_cast<cxsom::data::Map&>(arg_data).content);
	data_end   = data_begin + static_cast<cxsom::data::Map&>(arg_data).content.size();
      }
      else {
	// We never reach this.
      }

      return {data_begin, data_end};
    }
    
    // The same... but const.
    std::tuple<const double*, const double*> get_data_range(const cxsom::data::Base& arg_data) {
      const double* data_begin = nullptr;
      const double* data_end   = nullptr;

      // The data::Base class has a first_byte method that would have
      // avoided, the following if statements. Nevertheless, using it
      // would imply raw C-like casts.
      
      if(type->is_Scalar()) {
	data_begin = &(static_cast<const cxsom::data::Scalar&>(arg_data).value);
	data_end = data_begin + 1;
      }
      else if(type->is_Pos1D()) {
	data_begin = &(static_cast<const cxsom::data::d1::Pos&>(arg_data).x);
	data_end = data_begin + 1;
      }
      else if(type->is_Pos2D()) {
	data_begin = std::data(static_cast<const cxsom::data::d2::Pos&>(arg_data).xy);
	data_end   = data_begin + 2;
      }
      else if(type->is_Array()) {
	data_begin = std::data(static_cast<const cxsom::data::Array&>(arg_data).content);
	data_end   = data_begin + static_cast<const cxsom::data::Array&>(arg_data).content.size();
      }
      else if(type->is_Map()) {
	data_begin = std::data(static_cast<const cxsom::data::Map&>(arg_data).content);
	data_end   = data_begin + static_cast<const cxsom::data::Map&>(arg_data).content.size();
      }
      else {
	// We never reach this.
      }

      return {data_begin, data_end};
    }
    
    void update(std::vector<double>& mins, std::vector<double>& maxs, bool& meaningful, const cxsom::data::Base& arg_data) {
      
      auto [data_begin, data_end] = get_data_range(arg_data);
      
      if(!meaningful) {
	// mins and maxs have not been initialized yet.
	for(auto [it, min_it, max_it] = std::make_tuple(data_begin, mins.begin(), maxs.begin()); it != data_end; ++it, ++min_it, ++max_it) {
	  *min_it = *it;
	  *max_it = *it;
	}
	
	meaningful = true;
      }
      else 
	for(auto [it, min_it, max_it] = std::make_tuple(data_begin, mins.begin(), maxs.begin()); it != data_end; ++it, ++min_it, ++max_it) 
	  if      (*it < *min_it) *min_it = *it;
	  else if (*it > *max_it) *max_it = *it;
    }

  protected:

    // This is where you inherit callbacks for computation. See the
    // cxsom::update::Base class. You can read the pdf in the spec
    // section, the algorithm is described ("One update cycle for an
    // update u"). The point is that out-arguments are read
    // first. This can be re-tried until all of them are available
    // (the reading of out-arguments is aborted otherwise). Once this
    // is done, their value will not change, so their reading will not
    // occur anymore. The in-arguments, on the contrary, may vary
    // during the relaxation steps. This is why we separate the
    // computation related to out-arguments from the one related to
    // in-arguments. We will gather them at last, when the result is
    // ready to be written.

    virtual void on_computation_start() override {
      out_mins_maxs_meaningful = false; 
      in_mins_maxs_meaningful = false; 
    }
      
    virtual void on_read_out_arg(const cxsom::symbol::Instance&, unsigned int, const cxsom::data::Base& arg_data) override {
      are_there_out_args = true;
      update(out_mins, out_maxs, out_mins_maxs_meaningful, arg_data);
    }

    virtual void on_read_out_arg_aborted() override {
      out_mins_maxs_meaningful = false;
    }
    
    virtual void on_read_in_arg(const cxsom::symbol::Instance&, unsigned int, const cxsom::data::Base& arg_data) override {
      update(in_mins, in_maxs, in_mins_maxs_meaningful, arg_data);
    }
      
    virtual bool on_write_result(cxsom::data::Base& result_data) override {
      std::vector<double>::iterator mins_it, maxs_it;

      if(!are_there_out_args)
	// No out-arguments for this update. We have only in-arguments.
	std::tie(mins_it, maxs_it) = std::make_tuple(in_mins.begin(), in_maxs.begin());
      else if(!in_mins_maxs_meaningful)
	// Out-arguments, but no in-arguments. We have only out-arguments.
	std::tie(mins_it, maxs_it) = std::make_tuple(out_mins.begin(), out_maxs.begin());
      else {
	// Out- and in- arguments are used for this update.  we gather
	// the result of both in- and out- mins/maxs into in- stuff
	// (not out-, since out- may be re-used in a next relaxation
	// step).
	for(auto [in_it, out_it] = std::make_tuple(in_mins.begin(), out_mins.begin()); in_it != in_mins.end(); ++in_it, ++out_it)
	  if(*out_it < *in_it)
	    *in_it = *out_it;
	for(auto [in_it, out_it] = std::make_tuple(in_maxs.begin(), out_maxs.begin()); in_it != in_maxs.end(); ++in_it, ++out_it)
	  if(*out_it > *in_it)
	    *in_it = *out_it;
	std::tie(mins_it, maxs_it) = std::make_tuple(in_mins.begin(), in_maxs.begin());
      }

      // Ok, now the mins and maxs can be iterated from mins_it,
      // maxs_it. We only have to compute the mean of them and put the
      // values in te result.
      double max_diff = 0;
      auto [res_begin, res_end] = get_data_range(result_data);
      for(auto it = res_begin; it != res_end;) {
	double res = .5*(*(mins_it++) + *(maxs_it++));
	max_diff = std::max(max_diff, std::fabs(res - *it));
	*(it++) = res; // We set the result
      }
	  
      return max_diff > epsilon; // We return by signaling a significant change of the result.
    }
  };

  // The previous operation class will have to be notified to some
  // update factory. Let us write a function for registering all the
  // operations in foo (ok... only one here).
  void fill(cxsom::jobs::UpdateFactory& factory) {
    factory += {foo_MIDBOUND_NAME, cxsom::jobs::make_update_deterministic<MidBound>}; // we would have used make_update_random if the contructor had a "seed" argument.
  }

  
}
