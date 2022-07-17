#pragma once

#include <vector>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <memory>
#include <random>
#include <algorithm>
#include <iterator>
#include <array>
#include <limits>
#include <functional>
#include <map>
#include <cxsomData.hpp>
#include <cxsomUpdate.hpp>
#include <cxsomJobRule.hpp>

#include <fftconv.hpp>

using namespace std::placeholders;


// #define cxsomDEBUG_TOWARD_CONV_ARGMAX // For 1D only !
// #define cxsomDEBUG_CONVOLUTION        // For 1D only !
// #define cxsomDEBUG_CONVERGE

namespace cxsom {
  namespace error {
    struct empty_collection : public std::logic_error {
      using std::logic_error::logic_error;
    };
    
    struct already_existing_update : public std::logic_error {
      already_existing_update(const Operation& op) : std::logic_error(std::string("Update \"") + op + "\" already exists in the factory.") {}
    };
    
    struct not_existing_update : public std::logic_error {
      not_existing_update(const Operation& op) : std::logic_error(std::string("Update \"") + op + "\" does not exist in the factory.") {}
    };
  }

  namespace match {

    inline double gaussian(double a, double x, double y) {
      auto d = x - y;
      return std::exp(-a*(d*d));
    }

    inline double gaussian(unsigned int dim, double a, const double* xit, const double* yit) {
      auto x_end = xit + dim;
      double d2 = 0;
      while(xit != x_end) {
	double tmp = *(xit++) - *(yit++);
	d2 += tmp*tmp;
      }
      return std::exp(-a*d2);
    }

    inline double triangle(double a, double x, double y) {
      return std::max(0., 1.-a*std::fabs(x - y));
    }

    inline double triangle(unsigned int dim, double a, const double* xit, const double* yit) {
      auto x_end = xit + dim;
      double d2 = 0;
      while(xit != x_end) {
	double tmp = *(xit++) - *(yit++);
	d2 += tmp*tmp;
      }
      return std::max(0., 1.-a*std::sqrt(d2));
    }
    
  }
  
  namespace jobs {


    //////////////
    //          //
    // Job base //
    //          //
    //////////////

    class Base : public update::Base {
    protected:
      Operation op; // for debug...
    public:

      Base(data::Center& center,
	   const update::arg& res,
	   Operation op,
	   const std::vector<update::arg>& args)
	: update::Base(center, res, args), op(op) {}
      
      virtual std::string function_name() const override {
	std::ostringstream ostr;
	ostr << op;
	return ostr.str();
      }
    };
    

    template<typename UPDT>
    update::ref make_update_deterministic(data::Center& center,
					  const update::arg& res,
					  const std::vector<update::arg>& args,
					  const std::map<std::string, std::string>& params,
					  std::mt19937::result_type seed) {
      return std::shared_ptr<UPDT>(new UPDT(center, res, args, params));
    }
    
    template<typename UPDT>
    update::ref make_update_random(data::Center& center,
				   const update::arg& res,
				   const std::vector<update::arg>& args,
				   const std::map<std::string, std::string>& params,
				   std::mt19937::result_type seed) {
      return std::shared_ptr<UPDT>(new UPDT(center, res, args, params, seed));
    }
   
    //////////////
    //          //
    // Deadline // 
    //          //
    //////////////

    class Deadline {
    private:
      unsigned int ctr;
      unsigned int deadline;
      
    protected:
      
      void tick() {++ctr;};
      bool expired() {return ctr >= deadline;}

      Deadline(const std::map<std::string, std::string>& params)
	: ctr(0), deadline(100) {
	if(auto it = params.find("deadline"); it != params.end()) deadline = std::stoul(it->second);
      }

      unsigned int tick_time() const {return ctr;}
      
    public:
      Deadline() = delete;
    };

    //////////
    //      //
    // Copy //
    //      //
    //////////
    
    class Copy : public Base {
    private:
      std::vector<char> buffer;
      bool from_out;
      
    public:
      
      Copy(data::Center& center,
	   const update::arg& res,
	   const std::vector<update::arg>& args,
	   const std::map<std::string, std::string>&)
	: Base(center, res, "copy", args) {}
      
    protected:
      
      virtual void on_computation_start() override {
	from_out = false;
      }
      
      virtual void on_read_out_arg(const symbol::Instance&,
				   unsigned int,
				   const data::Base& data) override {
	buffer.resize(data.type->byte_length());
	data.write(std::data(buffer));
	from_out = true;
      }
  
      virtual void on_read_out_arg_aborted() override {}
    
      virtual void on_read_in_arg(const symbol::Instance&,
				  unsigned int,
				  const data::Base& data) {
	buffer.resize(data.type->byte_length());
	data.write(std::data(buffer));
      }
  
      virtual bool on_write_result(data::Base& data) override {
	if(from_out) {
	  data.read(std::data(buffer));
	  return true;
	}
	
	if(data.is_equal(std::data(buffer)))
	  return false;
	
	data.read(std::data(buffer));
	return true;
      }
    };
    

    /////////////
    //         //
    // Average //
    //         //
    /////////////
    
    class Average : public Base {
    private:
      std::vector<double> sum_out;
      unsigned int nb_out;
      bool sum_out_computed = false;
      std::vector<double> sum_in;
      unsigned int nb_in;
      type::ref type = nullptr;

      void check_internal_allocations(type::ref data_type) {
	if(!type) {
	  type = data_type;
	  // Types are checked. So this cast is ok.
	  auto& maptype = *(static_cast<const type::Map*>(type.get()));
	  sum_in.resize(maptype.size);
	  sum_out.resize(maptype.size);
	  std::fill(sum_in.begin(),  sum_in.end(),  0);
	  std::fill(sum_out.begin(), sum_out.end(), 0);
	}
      }
      
    public:
      
      double epsilon;
      
      Average(data::Center& center,
	      const update::arg& res,
	      const std::vector<update::arg>& args,
	      const std::map<std::string, std::string>& params)
	: Base(center, res, "average", args), epsilon(0) {
	if(auto it = params.find("epsilon"); it != params.end()) epsilon = std::stod(it->second);
      }
      
    protected:
      
      virtual void on_computation_start() override {
	if(!sum_out_computed) {
	  sum_out_computed = true;
	  std::fill(sum_out.begin(), sum_out.end(), 0);
	  nb_out = 0;
	}
	std::fill(sum_in.begin(), sum_in.end(), 0);
	nb_in = 0;
      }
      
      virtual void on_read_out_arg(const symbol::Instance&,
				   unsigned int,
				   const data::Base& data) override {
	check_internal_allocations(data.type);
	auto& content = static_cast<const data::Map&>(data).content;
	auto dit = content.begin();
	auto end = content.end();
	auto sit = sum_out.begin();
	while(dit != end) *(sit++) += *(dit++);
	++nb_out;
      }
  
      virtual void on_read_out_arg_aborted() override {
	sum_out_computed = false;
      }
    
      virtual void on_read_in_arg(const symbol::Instance&,
				  unsigned int,
				  const data::Base& data) override {
	check_internal_allocations(data.type);
	auto& content = static_cast<const data::Map&>(data).content;
	auto dit = content.begin();
	auto end = content.end();
	auto sit = sum_in.begin();
	while(dit != end) *(sit++) += *(dit++);
	++nb_in;
      }
  
      virtual bool on_write_result(data::Base& data) override {
	check_internal_allocations(data.type);
	
	unsigned int nb = nb_in + nb_out;
	if(nb == 0) return false;

	// We merge the two sums.
	std::vector<double>* accum_ptr = &sum_in;
	if(nb_in == 0) // all the data is in sum_out
	  accum_ptr = &sum_out;
	else if(nb_out != 0) {
	  auto soit = sum_out.begin();
	  auto end  = sum_out.end();
	  auto sit = accum_ptr->begin();
	  while(soit != end) *(sit++) += *(soit++);
	}

	// We compute the average.
	double coef = 1.0/nb;
	auto it  = accum_ptr->begin();
	auto end = accum_ptr->end();
	while(it != end) *(it++) *= coef;

	// We write the result and notify a significant change.
	double max_diff = 0;
	it  = accum_ptr->begin();
	end = accum_ptr->end();
	auto dit = static_cast<data::Map&>(data).content.begin();
	while(it != end) {
	  max_diff = std::max(max_diff, std::fabs(*it - *dit));
	  *(dit++) = *(it++);
	}


#ifdef cxsomDEBUG_CONVERGE
	std::ostringstream filename;
	filename << "avg-" << result.who.variable.timeline 
		 << "-" << result.who.variable.name
		 << "-" << std::setw(6) << std::setfill('0') << result.who.at
		 << ".data";
	std::ofstream file(filename.str().c_str(), std::ofstream::out | std::ofstream::app);
	file << static_cast<data::Map&>(data).content[0]
	     << " --> " << max_diff << " ? " << epsilon << std::endl;
	file.close();
#endif
	return max_diff > epsilon;
      }
    };
    

    ////////////
    //        //
    // Random //
    //        //
    ////////////
    
    class Random : public Base {
    private:
      
      std::mt19937 gen;
      
    public:

      Random(data::Center& center,
	     const update::arg& res,
	     const std::vector<update::arg>& args,
	     const std::map<std::string, std::string>&,
	     std::mt19937::result_type seed)
	: Base(center, res, "random", args), gen(seed) {}
      
    protected:
      
  
      virtual bool on_write_result(data::Base& data) override {
	auto u = std::uniform_real_distribution<double>(0, 1);
	if(data.type->is_Scalar()) 
	  static_cast<data::Scalar&>(data).value = u(gen);
	else if(data.type->is_Pos1D()) 
	  static_cast<data::d1::Pos&>(data).x = u(gen);
	else if(data.type->is_Pos2D()) 
	  static_cast<data::d2::Pos&>(data).xy = {u(gen), u(gen)};
	else if(auto size = data.type->is_Array(); size != 0)
	  for(auto& elem : static_cast<data::Array&>(data).content) elem = u(gen);
	else if(data.type->is_Map())
	  for(auto& elem : static_cast<data::Map&>(data).content) elem = u(gen);
	return true;
      }
    };

    

    //////////////
    //          //
    // Converge // 
    //          //   
    //////////////

    class Converge : public Base {
    private:
      double nb_converges;

#ifdef cxsomDEBUG_CONVERGE
      std::map<unsigned int, double> argval;
#endif
      
    public:

      
      Converge(data::Center& center,
	       const update::arg& res,
	       const std::vector<update::arg>& args,
	       const std::map<std::string, std::string>&)
	: Base(center, res, "converge", args), nb_converges(0) {
      }
      
    protected:

      
#ifdef cxsomDEBUG_CONVERGE
      virtual void on_read_in_arg(const symbol::Instance&, unsigned int arg_num, const data::Base& data) override {
	argval[arg_num] = static_cast<const data::Map&>(data).content[0];
      }
#endif
      
  
      virtual bool on_write_result(data::Base& data) override {
	static_cast<data::Scalar&>(data).value = ++nb_converges;

#ifdef cxsomDEBUG_CONVERGE
	std::ostringstream filename;
	filename << "cgce-" << result.who.variable.timeline 
		 << "-" << result.who.variable.name
		 << "-" << std::setw(6) << std::setfill('0') << result.who.at
		 << ".data";
	std::ofstream file(filename.str().c_str(), std::ofstream::out | std::ofstream::app);
	for(auto& kv : argval)
	  file << kv.first << ':' << kv.second << ' ';
	file << std::endl;
	file.close();
#endif
	
	return true;
      }
    };

    
    ///////////
    //       //
    // Clear // 
    //       //   
    ///////////
    
    class Clear : public Base {
    private:
      double value;
      
    public:

      Clear(data::Center& center,
	    const update::arg& res,
	    const std::vector<update::arg>& args,
	    const std::map<std::string, std::string>& params)
	: Base(center, res, "clear", args), value(0) {
	if(auto it = params.find("value"); it != params.end()) value = std::stod(it->second);
      }
      
    protected:
      
  
      virtual bool on_write_result(data::Base& data) override {
	if(data.type->is_Scalar()) 
	  static_cast<data::Scalar&>(data).value = value;
	else if(data.type->is_Pos1D()) 
	  static_cast<data::d1::Pos&>(data).x = value;
	else if(data.type->is_Pos2D()) 
	  static_cast<data::d2::Pos&>(data).xy = {value, value};
	else if(data.type->is_Array()) {
	  auto& content = static_cast<data::Array&>(data).content;
	  std::fill(content.begin(), content.end(), value);
	}
	else if(data.type->is_Map()) {
	  auto& content = static_cast<data::Map&>(data).content;
	  std::fill(content.begin(), content.end(), value);
	}
	return true;
      }
    };

    ///////////
    //       //
    // Merge // 
    //       //   
    ///////////
    
    class Merge : public Base {
    private:
      double beta, beta_;
      double epsilon;
      std::vector<double> ae, ac;
      type::ref type = nullptr;
      
      void check_internal_allocations(type::ref data_type) {
	if(!type) {
	  type = data_type;
	  // Types are checked. So this cast is ok.
	  auto& maptype = *(static_cast<const type::Map*>(type.get()));
	  ae.resize(maptype.size);
	  ac.resize(maptype.size);
	  std::fill(ae.begin(), ae.end(), 0);
	  std::fill(ac.begin(), ac.end(), 0);
	}
      }
      
    public:

      Merge(data::Center& center,
	    const update::arg& res,
	    const std::vector<update::arg>& args,
	    const std::map<std::string, std::string>& params)
	: Base(center, res, "merge", args), beta(.5), beta_(.5),  epsilon(0) {
	if(auto it = params.find("beta")   ; it != params.end()) {
	  beta  = std::stod(it->second);
	  beta_ = 1 - beta;
	}
	if(auto it = params.find("epsilon"); it != params.end()) epsilon = std::stod(it->second);
      }
      
    protected:
            
      virtual void on_read_out_arg(const symbol::Instance&, unsigned int argnum, const data::Base& data) override {
	check_internal_allocations(data.type);
	auto& content = static_cast<const data::Map&>(data).content;
	if(argnum == 0) std::copy(content.begin(), content.end(), ae.begin());
	else            std::copy(content.begin(), content.end(), ac.begin());
      }

      virtual void on_read_in_arg(const symbol::Instance&, unsigned int argnum, const data::Base& data) override {
	check_internal_allocations(data.type);
	auto& content = static_cast<const data::Map&>(data).content;
	if(argnum == 0) std::copy(content.begin(), content.end(), ae.begin());
	else            std::copy(content.begin(), content.end(), ac.begin());
      }
      
      virtual bool on_write_result(data::Base& data ) override {
	auto& content = static_cast<data::Map&>(data).content;
	auto eit = ae.begin();
	auto cit = ac.begin();
	double max_diff = 0;
	for(auto& a : content) {
	  double a_tmp = std::sqrt(*eit * (beta * *eit + beta_ * *(cit++)));
	  max_diff = std::max(max_diff, std::fabs(a_tmp - a));
	  a = a_tmp;
	  ++eit;
	}
	  
	return max_diff > epsilon;
      }
      
    };

    ///////////
    //       //
    // Match // 
    //       //   
    ///////////
    
    class Match : public Base {
    protected:
      double epsilon;
      std::vector<char> w;
      double                i1;
      std::array<double, 2> i2;
      std::vector<double>   in;
      type::ref input_type = nullptr;
      
      void read_data(unsigned int argnum, const data::Base& data) {
	if(argnum == 0) {
	  input_type = data.type;
	  if     (input_type->is_Scalar()) i1 = static_cast<const data::Scalar&>(data).value;
	  else if(input_type->is_Pos1D())  i1 = static_cast<const data::d1::Pos&>(data).x;
	  else if(input_type->is_Pos2D())  i2 = static_cast<const data::d2::Pos&>(data).xy;
	  else if(auto size = input_type->is_Array(); size > 0) {
	    in.resize(size);
	    auto& content = static_cast<const data::Array&>(data).content;
	    std::copy(content.begin(), content.end(), in.begin());
	  }
	  else
	    throw std::runtime_error("Match : input type is bad, while it has been checked");
	}
	else {
	  w.resize(data.type->byte_length());
	  data.write(std::data(w));
	}
      }
      
    public:

      Match(data::Center& center,
	    const update::arg& res,
	    const std::vector<update::arg>& args,
	    const std::map<std::string, std::string>& params,
	    Operation op)
	: Base(center, res, op, args), epsilon(0) {
	if(auto it = params.find("epsilon"); it != params.end())  epsilon = std::stod(it->second);
      }
      
    protected:
      
      virtual void on_read_out_arg(const symbol::Instance&, unsigned int argnum, const data::Base& data) override {
	read_data(argnum, data);
      }
      
      virtual void on_read_in_arg(const symbol::Instance&, unsigned int argnum, const data::Base& data) override {
	read_data(argnum, data);
      }
      
    };

    ///////////////////
    //               //
    // MatchTriangle // 
    //               //   
    ///////////////////
    
    class MatchTriangle : public Match {
    private:
      
      double _r;
      
    public:

      MatchTriangle(data::Center& center,
		    const update::arg& res,
		    const std::vector<update::arg>& args,
		    const std::map<std::string, std::string>& params)
	: Match(center, res, args, params, "match-triangle"), _r(2) {
	if(auto it = params.find("r"); it != params.end()) _r = 1/std::stod(it->second);
      }
      
    protected:
      
      virtual bool on_write_result(data::Base& data) override {
	double max_diff = 0;
	auto& content   = static_cast<data::Map&>(data).content;
	if(input_type->is_Scalar() || input_type->is_Pos1D()) {
	  double* weights = reinterpret_cast<double*>(std::data(w));
	  for(auto& res : content) {
	    double tmp =  match::triangle(_r, *(weights++), i1);
	    max_diff = std::max(max_diff, std::fabs(tmp - res));
	    res = tmp;
	  }
	}
	else if(input_type->is_Pos2D()) {
	  double* weights = reinterpret_cast<double*>(std::data(w));
	  for(auto& res : content) {
	    double tmp =  match::triangle(2, _r, weights, std::data(i2));
	    weights += 2;
	    max_diff = std::max(max_diff, std::fabs(tmp - res));
	    res = tmp;
	  }
	}
	else if(auto size = input_type->is_Array(); size > 0) {
	  double* weights = reinterpret_cast<double*>(std::data(w));
	  for(auto& res : content) {
	    double tmp =  match::triangle(size, _r, weights, std::data(in));
	    weights += size;
	    max_diff = std::max(max_diff, std::fabs(tmp - res));
	    res = tmp;
	  }
	}
	else
	  throw std::runtime_error("MatchTriangle : input type is bad, while it has been checked");
	
	  
	return max_diff > epsilon;
      }
      
    };

    ///////////////////
    //               //
    // MatchGaussian // 
    //               //   
    ///////////////////
    
    class MatchGaussian : public Match {
    private:
      double _2sigma2;
      
    public:

      MatchGaussian(data::Center& center,
		    const update::arg& res,
		    const std::vector<update::arg>& args,
		    const std::map<std::string, std::string>& params)
	: Match(center, res, args, params, "match-gaussian"), _2sigma2(.5) {
	if(auto it = params.find("sigma")   ; it != params.end()) {
	  double tmp = std::stod(it->second);
	  _2sigma2  = .5/(tmp * tmp);
	}
      }
      
    protected:
      
      virtual bool on_write_result(data::Base& data) override {
	double max_diff = 0;
	auto& content   = static_cast<data::Map&>(data).content;
	if(input_type->is_Scalar() || input_type->is_Pos1D()) {
	  double* weights = reinterpret_cast<double*>(std::data(w));
	  for(auto& res : content) {
	    double tmp =  match::gaussian(_2sigma2, *(weights++), i1);
	    max_diff = std::max(max_diff, std::fabs(tmp - res));
	    res = tmp;
	  }
	}
	else if(input_type->is_Pos2D()) {
	  double* weights = reinterpret_cast<double*>(std::data(w));
	  for(auto& res : content) {
	    double tmp =  match::gaussian(2, _2sigma2, weights, std::data(i2));
	    weights += 2;
	    max_diff = std::max(max_diff, std::fabs(tmp - res));
	    res = tmp;
	  }
	}
	else if(auto size = input_type->is_Array(); size > 0) {
	  double* weights = reinterpret_cast<double*>(std::data(w));
	  for(auto& res : content) {
	    double tmp =  match::gaussian(size, _2sigma2, weights, std::data(in));
	    weights += size;
	    max_diff = std::max(max_diff, std::fabs(tmp - res));
	    res = tmp;
	  }
	}
	else
	  throw std::runtime_error("MatchGaussian : input type is bad, while it has been checked");
	
	  
	return max_diff > epsilon;
      }
      
    };



    ///////////
    //       //
    // Learn // 
    //       //   
    ///////////
    
    class Learn : public Base {
    protected:
      double epsilon;
      double alpha;
      double tol;
      std::vector<char> w;
      double                bmu1;
      std::array<double, 2> bmu2;
      std::vector<double>   bmun;
      double                i1;
      std::array<double, 2> i2;
      std::vector<double>   in;
      type::ref input_type  = nullptr;
      type::ref weight_type = nullptr;
      type::ref bmu_type    = nullptr;
      
      void read_data(unsigned int argnum, const data::Base& data) {
	switch(argnum) {
	case 0 : // input
	  input_type = data.type;
	  if     (input_type->is_Scalar()) i1 = static_cast<const data::Scalar&>(data).value;
	  else if(input_type->is_Pos1D())  i1 = static_cast<const data::d1::Pos&>(data).x;
	  else if(input_type->is_Pos2D())  i2 = static_cast<const data::d2::Pos&>(data).xy;
	  else if(auto size = input_type->is_Array(); size > 0) {
	    in.resize(size);
	    auto& content = static_cast<const data::Array&>(data).content;
	    std::copy(content.begin(), content.end(), in.begin());
	  }
	  else
	    throw std::runtime_error("Learn : input type is bad, while it has been checked");
	  break;
	case 1 : // weights
	  weight_type = data.type;
	  w.resize(weight_type->byte_length());
	  data.write(std::data(w));
	  break;
	case 2 : // BMU
	  bmu_type = data.type;
	  if     (bmu_type->is_Pos1D()) bmu1 = static_cast<const data::d1::Pos&>(data).x;
	  else if(bmu_type->is_Pos2D()) bmu2 = static_cast<const data::d2::Pos&>(data).xy;
	  else if(auto size = bmu_type->is_Array(); size > 0) {
	    bmun.resize(size);
	    auto& content = static_cast<const data::Array&>(data).content;
	    std::copy(content.begin(), content.end(), bmun.begin());
	  }
	  else
	    throw std::runtime_error("Learn : input type is bad, while it has been checked");
	  break;
	}
      }
      
    public:

      Learn(data::Center& center,
	    const update::arg& res,
	    const std::vector<update::arg>& args,
	    const std::map<std::string, std::string>& params,
	    Operation op)
	: Base(center, res, op, args), epsilon(0), alpha(.05), tol(1e-5) {
	if(auto it = params.find("epsilon"); it != params.end()) epsilon = std::stod(it->second);
	if(auto it = params.find("alpha");   it != params.end()) alpha   = std::stod(it->second);
	if(auto it = params.find("tol");     it != params.end()) tol     = std::stod(it->second);
      }
      
    protected:
      
      virtual void on_read_out_arg(const symbol::Instance&, unsigned int argnum, const data::Base& data) override {
	read_data(argnum, data);
      }
      
      virtual void on_read_in_arg(const symbol::Instance&, unsigned int argnum, const data::Base& data) override {
	read_data(argnum, data);
      }
      
      template<typename MatchFun>
      double learn_W1D_I1D(unsigned int size, double coef, double* res, double xi, double* weights, const MatchFun& match_fun) {
	double max_diff = 0;
	double i = 0;
	const double* end = res + size;
	while(res != end) {
	  double w = *(weights++);
	  if(double h = match_fun(coef*i++); h != 0) {
	    if(double k = h * alpha; k > tol) {
	      double tmp = (1-k) * w + k * xi;
	      max_diff = std::max(max_diff, std::fabs(tmp - *res));
	      *(res++) = tmp;
	    }
	    else
	      *(res++) = w;
	  }
	  else
	    *(res++) = w;
	}
	return max_diff;    
      }

      template<typename MatchFun>
      double learn_W1D_I2D(unsigned int size, double coef, double* res, double* xi, double* weights, const MatchFun& match_fun) {
	double max_diff = 0;
	double i = 0;
	const double* end = res + 2 * size;
	double xi0 = *(xi++);
	double xi1 = *xi;
	double* res0 = res++;
	double* res1 = res;
	for(; res0 != end; res0 = res1+1, res1 = res0+1) {
	  double w0 = *(weights++);
	  double w1 = *(weights++);
	  if(double h = match_fun(coef*i++); h != 0) {
	    if(double k = h * alpha; k > tol) {
	      double tmp = (1-k) * w0 + k * xi0;
	      max_diff = std::max(max_diff, std::fabs(tmp - *res0));
	      *res0 = tmp;
	      tmp = (1-k) * w1 + k * xi1;
	      max_diff = std::max(max_diff, std::fabs(tmp - *res1));
	      *res1 = tmp;
	    }
	    else {
	      *res0 = w0;
	      *res1 = w1;
	    }
	  }
	  else {
	    *res0 = w0;
	    *res1 = w1;
	  }
	}
	return max_diff;    
      }

      template<typename MatchFun>
      double learn_W1D_InD(unsigned int size, unsigned int dim, double coef, double* res, double* xi, double* weights, const MatchFun& match_fun) {
	double max_diff = 0;
	double i = 0;
	const double* end = res + dim * size;
	while(res != end) {
	  double* dim_end = res + dim;
	  if(double h = match_fun(coef*i++); h != 0) {
	    if(double k = h * alpha; k > tol) {
	      double* xit = xi;
	      while(res != dim_end) {
		double tmp = (1-k) * *(weights++) + k * *(xit++);
		max_diff = std::max(max_diff, std::fabs(tmp - *res));
		*(res++) = tmp;
	      }
	    }
	    else
	      while(res != dim_end) *(res++) = *(weights++);
	  }
	  else
	    while(res != dim_end) *(res++) = *(weights++);
	}
	return max_diff;    
      }

      template<typename MatchFun>
      double learn_W2D_I1D(unsigned int side, double coef, double* res, double xi, double* weights, const MatchFun& match_fun) {
	double max_diff = 0;
	std::array<double, 2> xy;
	for(unsigned int i = 0; i < side; ++i) {
	  xy[1] = i*coef;
	  for(unsigned int j = 0; j < side; ++j, ++res) {
	    xy[0] = j*coef;
	    double w = *(weights++);
	    if(double h = match_fun(std::data(xy)); h != 0) {
	      if(double k   = h * alpha; k > tol) {
		double tmp = (1-k) * w + k * xi;
		max_diff = std::max(max_diff, std::fabs(tmp - *res));
		*res = tmp;
	      }
	      else
		*res = w;
	    }
	    else
	      *res = w;
	  }
	}
	return max_diff;    
      }
      
      template<typename MatchFun>
      double learn_W2D_I2D(unsigned int side, double coef, double* res, double* xi, double* weights, const MatchFun& match_fun) {
	double max_diff = 0;
	std::array<double, 2> xy;
	double xi0 = *(xi++);
	double xi1 = *xi;
	double* res0 = res++;
	double* res1 = res;
	for(unsigned int i = 0; i < side; ++i) {
	  xy[1] = i*coef;
	  for(unsigned int j = 0; j < side; ++j, res0 = res1+1, res1 = res0+1) {
	    xy[0] = j*coef;
	    double w0 = *(weights++);
	    double w1 = *(weights++);
	    if(double h = match_fun(std::data(xy)); h != 0) {
	      if(double k   = h * alpha; k > tol) {
		double tmp = (1-k) * w0 + k * xi0;
		max_diff = std::max(max_diff, std::fabs(tmp - *res0));
		*res0 = tmp;
		tmp = (1-k) * w1 + k * xi1;
		max_diff = std::max(max_diff, std::fabs(tmp - *res1));
		*res1 = tmp;
	      }
	      else {
		*res0 = w0;
		*res1 = w1;
	      }
	    }
	    else {
	      *res0 = w0;
	      *res1 = w1;
	    }
	  }
	}
	return max_diff;    
      }

      template<typename MatchFun>
      double learn_W2D_InD(unsigned int side, unsigned int dim, double coef, double* res, double* xi, double* weights, const MatchFun& match_fun) {
	double max_diff = 0;
	std::array<double, 2> xy;
	for(unsigned int i = 0; i < side; ++i) {
	  xy[1] = i*coef;
	  for(unsigned int j = 0; j < side; ++j) {
	    xy[0] = j*coef;
	    double* dim_end = res + dim;
	    if(double h = match_fun(std::data(xy)); h != 0) {
	      if(double k = h * alpha; k > tol) {
		double* xit = xi;
		while(res != dim_end) {
		  double tmp = (1-k) * *(weights++) + k * *(xit++);
		  max_diff = std::max(max_diff, std::fabs(tmp - *res));
		  *(res++) = tmp;
		}
	      }
	      else
		while(res != dim_end) *(res++) = *(weights++);
	    }
	    else
	      while(res != dim_end) *(res++) = *(weights++);
	  }
	}

	return max_diff;    
      }

    };

    ///////////////////
    //               //
    // LearnTriangle // 
    //               //   
    ///////////////////
    
    class LearnTriangle : public Learn {
    private:
      
      double _r;
      
    public:

      LearnTriangle(data::Center& center,
		    const update::arg& res,
		    const std::vector<update::arg>& args,
		    const std::map<std::string, std::string>& params)
	: Learn(center, res, args, params, "learn-triangle"), _r(2) {
	if(auto it = params.find("r"); it != params.end()) _r = 1/std::stod(it->second);
      }
      
    protected:
      
      virtual bool on_write_result(data::Base& data) override {
	double       max_diff = 0;
	auto&        map_type = *(static_cast<const type::Map*>(weight_type.get()));
	double       coef     = 1.0/(map_type.side - 1.0);
	auto&        content  = static_cast<data::Map&>(data).content;
	double*      weights  = reinterpret_cast<double*>(std::data(w));
	if(weight_type->is_Map1D()) {
	  if(input_type->is_Scalar() || input_type->is_Pos1D()) max_diff = learn_W1D_I1D(map_type.size,      coef, std::data(content), i1,            weights, [h = _r, bmu = bmu1](double p) {return match::triangle(h, p, bmu);});
	  else if(input_type->is_Pos2D())                       max_diff = learn_W1D_I2D(map_type.size,      coef, std::data(content), std::data(i2), weights, [h = _r, bmu = bmu1](double p) {return match::triangle(h, p, bmu);});
	  else if(auto dim = input_type->is_Array(); dim > 0)   max_diff = learn_W1D_InD(map_type.size, dim, coef, std::data(content), std::data(in), weights, [h = _r, bmu = bmu1](double p) {return match::triangle(h, p, bmu);});
	  else
	    throw std::runtime_error("LearnTriangle : input type is bad, while it has been checked");
	}
	else { // weights is Map 2D
	  if(input_type->is_Scalar() || input_type->is_Pos1D()) max_diff = learn_W2D_I1D(map_type.side,      coef, std::data(content), i1,            weights, [h = _r, bmu = std::data(bmu2)](double* p) {return match::triangle(2, h, p, bmu);});
	  else if(input_type->is_Pos2D())                       max_diff = learn_W2D_I2D(map_type.side,      coef, std::data(content), std::data(i2), weights, [h = _r, bmu = std::data(bmu2)](double* p) {return match::triangle(2, h, p, bmu);});
	  else if(auto dim = input_type->is_Array(); dim > 0)   max_diff = learn_W2D_InD(map_type.side, dim, coef, std::data(content), std::data(in), weights, [h = _r, bmu = std::data(bmu2)](double* p) {return match::triangle(2, h, p, bmu);});
	  else
	    throw std::runtime_error("LearnTriangle : input type is bad, while it has been checked");
	}
	  
	return max_diff > epsilon;
      }
      
    };

    ///////////////////
    //               //
    // LearnGaussian // 
    //               //   
    ///////////////////
    
    class LearnGaussian : public Learn {
    private:
      
      double _2sigma2;
      
    public:

      LearnGaussian(data::Center& center,
		    const update::arg& res,
		    const std::vector<update::arg>& args,
		    const std::map<std::string, std::string>& params)
	: Learn(center, res, args, params, "learn_gaussian"), _2sigma2(.5) {
	if(auto it = params.find("sigma")   ; it != params.end()) {
	  double tmp = std::stod(it->second);
	  _2sigma2  = .5/(tmp * tmp);
	}
      }
      
    protected:
      
      virtual bool on_write_result(data::Base& data) override {
	double       max_diff = 0;
	auto&        map_type = *(static_cast<const type::Map*>(weight_type.get()));
	double       coef     = 1.0/(map_type.side - 1.0);
	auto&        content  = static_cast<data::Map&>(data).content;
	double*      weights  = reinterpret_cast<double*>(std::data(w));
	if(weight_type->is_Map1D()) {
	  if(input_type->is_Scalar() || input_type->is_Pos1D()) max_diff = learn_W1D_I1D(map_type.size,      coef, std::data(content), i1,            weights, [h = _2sigma2, bmu = bmu1](double p) {return match::gaussian(h, p, bmu);});
	  else if(input_type->is_Pos2D())                       max_diff = learn_W1D_I2D(map_type.size,      coef, std::data(content), std::data(i2), weights, [h = _2sigma2, bmu = bmu1](double p) {return match::gaussian(h, p, bmu);});
	  else if(auto dim = input_type->is_Array(); dim > 0)   max_diff = learn_W1D_InD(map_type.size, dim, coef, std::data(content), std::data(in), weights, [h = _2sigma2, bmu = bmu1](double p) {return match::gaussian(h, p, bmu);});
	  else
	    throw std::runtime_error("LearnGaussian : input type is bad, while it has been checked");
	}
	else { // weights is Map 2D
	  if(input_type->is_Scalar() || input_type->is_Pos1D()) max_diff = learn_W2D_I1D(map_type.side,      coef, std::data(content), i1,            weights, [h = _2sigma2, bmu = std::data(bmu2)](double* p) {return match::gaussian(2, h, p, bmu);});
	  else if(input_type->is_Pos2D())                       max_diff = learn_W2D_I2D(map_type.side,      coef, std::data(content), std::data(i2), weights, [h = _2sigma2, bmu = std::data(bmu2)](double* p) {return match::gaussian(2, h, p, bmu);});
	  else if(auto dim = input_type->is_Array(); dim > 0)   max_diff = learn_W2D_InD(map_type.side, dim, coef, std::data(content), std::data(in), weights, [h = _2sigma2, bmu = std::data(bmu2)](double* p) {return match::gaussian(2, h, p, bmu);});
	  else
	    throw std::runtime_error("LearnGaussian : input type is bad, while it has been checked");
	}
	  
	return max_diff > epsilon;
      }
      
    };

    
    ////////////////
    //            //
    // ArgmaxBase // 
    //            //
    ////////////////
    
    class ArgmaxBase : public Base, protected Deadline {
      
    protected:
      mutable std::mt19937 gen;
      double epsilon;
      bool random_bmu;
      double argmax_tol;
      
      
      std::size_t side, size;
      double coef;

      
      ArgmaxBase(data::Center& center,
		 const update::arg& res,
		 Operation op,
		 const std::vector<update::arg>& args,
		 const std::map<std::string, std::string>& params,
		 std::mt19937::result_type seed)
	: Base(center, res, op, args), Deadline(params), gen(seed), epsilon(0), random_bmu(true), argmax_tol(1e-8) {
	if(auto it = params.find("epsilon");    it != params.end()) epsilon    = std::stod(it->second);
	if(auto it = params.find("argmax-tol"); it != params.end()) argmax_tol = std::stod(it->second);
	if(auto it = params.find("random-bmu"); it != params.end()) random_bmu = (std::stod(it->second) != 0);
      }

      std::size_t find_argmax(const double* begin, std::size_t size) const {
	if(random_bmu) {
	  std::vector<std::size_t> bmus;
	  double max         = std::numeric_limits<double>::lowest();
	  double max_tol_inf = std::numeric_limits<double>::lowest();
	  double max_tol_sup = max + argmax_tol;
	  auto it = begin;
	  auto end = begin + size;
	  for(;it != end; ++it) 
	    if(double v = *it++; v > max_tol_inf) { 
	      if(v > max_tol_sup) {
		bmus.clear();
		max         = v;
		max_tol_inf = max - argmax_tol;
		max_tol_sup = max + argmax_tol;
	      }
	      bmus.push_back(std::distance(begin, it));
	    }
	  if(bmus.size() == 0)
	    throw error::empty_collection("ArgmaxBase cannot find argmax of an empty collection");
	  
	  return bmus[std::uniform_int_distribution<std::size_t>(0, bmus.size() - 1)(gen)];
	}
	else
	  return std::distance(begin, std::max_element(begin, begin + size));
      }
    };
    
    ////////////
    //        //
    // Argmax // 
    //        //
    ////////////
    
    class Argmax : public ArgmaxBase {
    private:

      bool is_out;
      type::ref arg_type = nullptr;
      double bmu1;
      std::array<double, 2> bmu2;

      void read_data(const data::Base& data) {
	if(!arg_type) {
	  arg_type = data.type;
	  side = static_cast<const type::Map*>(arg_type.get())->side;
	  size = static_cast<const type::Map*>(arg_type.get())->size;
	  coef = 1/(side - 1.0);
	}
	auto argmax = find_argmax(std::data(static_cast<const data::Map&>(data).content), size);
	if(arg_type->is_Map1D()) bmu1 = argmax*coef;
	else                     bmu2 = {(argmax % side)*coef, (argmax / side)*coef};
      }
      
    public:

      Argmax(data::Center& center,
	     const update::arg& res,
	     const std::vector<update::arg>& args,
	     const std::map<std::string, std::string>& params,
	     std::mt19937::result_type seed)
	: ArgmaxBase(center, res, "argmax", args, params, seed) {
      }
      
    protected:
  
      virtual void on_read_out_arg(const symbol::Instance&, unsigned int, const data::Base& data) override {
	is_out = true;
	read_data(data);
      }
      
      virtual void on_read_in_arg(const symbol::Instance&, unsigned int, const data::Base& data) override {
	is_out = false;
	read_data(data);
      }
      
      virtual bool on_write_result(data::Base& data) override {
	tick();
	if(data.type->is_Pos1D()) {
	  if(is_out) {
	    static_cast<data::d1::Pos&>(data).x = bmu1;
	    return !expired();
	  }
	  else if(std::fabs(static_cast<data::d1::Pos&>(data).x - bmu1) > epsilon) {
	    static_cast<data::d1::Pos&>(data).x = bmu1;
	    return !expired();
	  }
	  else {
	    static_cast<data::d1::Pos&>(data).x = bmu1;
	    return false;
	  }
	}
	else {
	  if(is_out) {
	    static_cast<data::d2::Pos&>(data).xy = bmu2;
	    return !expired();
	  }
	  else if(std::max(std::fabs(static_cast<data::d2::Pos&>(data).xy[0] - bmu2[0]),
			   std::fabs(static_cast<data::d2::Pos&>(data).xy[1] - bmu2[1])) > epsilon) {
	    static_cast<data::d2::Pos&>(data).xy = bmu2;
	    return !expired();
	  }
	  else {
	    static_cast<data::d2::Pos&>(data).xy = bmu2;
	    return false;
	  }
	}
      }
    };

   
    ////////////////
    //            //
    // ConvArgmax // 
    //            //
    ////////////////
    
    class ConvArgmax : public ArgmaxBase {
    private:
      double sigma;

      bool is_out;
      std::shared_ptr<fftconv::Convolution> conv_ptr = nullptr;
      std::vector<double> data_content;
      type::ref arg_type = nullptr;

      void read_data(const data::Base& data) {
	auto& vec = static_cast<const data::Map&>(data).content;
	data_content.resize(vec.size());
	std::copy(vec.begin(), vec.end(), data_content.begin());
	
	if(!conv_ptr) {
	  arg_type = data.type;
	  side = static_cast<const type::Map*>(arg_type.get())->side;
	  size = static_cast<const type::Map*>(arg_type.get())->size;
	  coef = 1/(side - 1.0);
	  unsigned int h = 1;
	  if(arg_type->is_Map2D()) h = (unsigned int)side;
	  conv_ptr = std::make_shared<fftconv::Convolution>((unsigned int)side, h, sigma*side,
							    data_content,
							    fftconv::KernelType::Gaussian,
							    fftconv::PaddingType::Constant);
	}
	conv_ptr->convolve();
      }
      
      void read_data(const data::Base& data, unsigned int at) {
	read_data(data);
	std::ostringstream filename;
	filename << "debug-convolution-at-" << std::setfill('0') << std::setw(6) << at << ".gnuplot";
	std::cout << "Writing \"" << filename.str() << "\"" << std::endl;
	std::ofstream file(filename.str().c_str());
	file << "plot '-' using 1 with lines, '-' using 1 with lines" << std::endl;
	double* cit = conv_ptr->ws.dst;
	double* cit_end = cit + size;
	for(auto s : static_cast<const data::Map&>(data).content) file << s << std::endl;
	file << 'e' << std::endl;
	while(cit != cit_end)                                             file << *(cit++) << std::endl;
      }
      
    public:

      ConvArgmax(data::Center& center,
		 const update::arg& res,
		 const std::vector<update::arg>& args,
		 const std::map<std::string, std::string>& params,
		 std::mt19937::result_type seed)
	: ArgmaxBase(center, res, "conv-argmax", args, params, seed), sigma(.1) {
	if(auto it = params.find("sigma");   it != params.end()) sigma   = std::stod(it->second);
      }
      
    protected:

      
#ifdef cxsomDEBUG_CONVOLUTION
      virtual void on_read_out_arg(const symbol::Instance& i, unsigned int, const data::Base& data) override {
	is_out = true;
	read_data(data, i.at);
      }
      
      virtual void on_read_in_arg(const symbol::Instance& i, unsigned int, const data::Base& data) override {
	is_out = false;
	read_data(data, i.at);
      }
#else
      virtual void on_read_out_arg(const symbol::Instance&, unsigned int, const data::Base& data) override {
	is_out = true;
	read_data(data);
      }
      
      virtual void on_read_in_arg(const symbol::Instance&, unsigned int, const data::Base& data) override {
	is_out = false;
	read_data(data);
      }
#endif
      
      virtual bool on_write_result(data::Base& data) override {
	tick();
	auto argmax = find_argmax(conv_ptr->ws.dst, size);
	if(data.type->is_Pos1D()) {
	  double bmu = argmax*coef;
	  if(is_out) {
	    static_cast<data::d1::Pos&>(data).x = bmu;
	    return !expired();
	  }
	  else if(std::fabs(static_cast<data::d1::Pos&>(data).x - bmu) > epsilon) {
	    static_cast<data::d1::Pos&>(data).x = bmu;
	    return !expired();
	  }
	  else {
	    static_cast<data::d1::Pos&>(data).x = bmu;
	    return false;
	  }
	}
	else {
	  std::array<double, 2> bmu {(argmax % side)*coef, (argmax / side)*coef};
	  if(is_out) {
	    static_cast<data::d2::Pos&>(data).xy = bmu;
	    return !expired();
	  }
	  else if(std::max(std::fabs(static_cast<data::d2::Pos&>(data).xy[0] - bmu[0]),
			   std::fabs(static_cast<data::d2::Pos&>(data).xy[1] - bmu[1])) > epsilon) {
	    static_cast<data::d2::Pos&>(data).xy = bmu;
	    return !expired();
	  }
	  else {
	    static_cast<data::d2::Pos&>(data).xy = bmu;
	    return false;
	  }
	}
      }
    };

   
   
    //////////////////
    //              //
    // TowardArgmax // 
    //              //
    //////////////////

    class TowardArgmax : public ArgmaxBase {
    private:
      double delta;
      double delta2;

      std::vector<double> data_content;
      type::ref map_type = nullptr;
      double current1;
      std::array<double, 2> current2;      

      void read_current(const data::Base& data) {
	if(data.type->is_Pos1D())
	  current1 = static_cast<const data::d1::Pos&>(data).x;
	else
	  current2 = static_cast<const data::d2::Pos&>(data).xy;
      }
      
      void read_map(const data::Base& data) {
	auto& vec = static_cast<const data::Map&>(data).content;
	data_content.resize(vec.size());
	std::copy(vec.begin(), vec.end(), data_content.begin());

	if(!map_type) {
	  map_type = data.type;
	  side = static_cast<const type::Map*>(map_type.get())->side;
	  size = static_cast<const type::Map*>(map_type.get())->size;
	  coef = 1/(side - 1.0);
	}
      }
      
    public:

      TowardArgmax(data::Center& center,
		       const update::arg& res,
		       const std::vector<update::arg>& args,
		       const std::map<std::string, std::string>& params,
		       std::mt19937::result_type seed)
	: ArgmaxBase(center, res, "toward-argmax", args, params, seed), delta(.05) {
	if(auto it = params.find("delta");   it != params.end()) delta   = std::stod(it->second);
	delta2 = delta*delta;
      }
      
    protected:
  
      virtual void on_read_out_arg(const symbol::Instance&, unsigned int arg_num, const data::Base& data) override {
	switch(arg_num) {
	case 0 : read_map(data);     break;
	case 1 : read_current(data); break;
	}
      }
      
      virtual void on_read_in_arg(const symbol::Instance&, unsigned int arg_num, const data::Base& data) override {
	switch(arg_num) {
	case 0 : read_map(data);     break;
	case 1 : read_current(data); break;
	}
      }
      
      virtual bool on_write_result(data::Base& data) override {
	tick();
	auto argmax = find_argmax(std::data(data_content), size);
	if(data.type->is_Pos1D()) {
	  double bmu = argmax*coef;
	  double d = std::fabs(bmu - current1);
	  double tmp;
	  if(d <= delta)          tmp = bmu;
	  else if(current1 > bmu) tmp = current1 - delta;
	  else                    tmp = current1 + delta;
	  
	  if(std::fabs(static_cast<data::d1::Pos&>(data).x - tmp) > epsilon) {
	    static_cast<data::d1::Pos&>(data).x = tmp;
	    return !expired();
	  }
	  else {
	    static_cast<data::d1::Pos&>(data).x = tmp;
	    return false;
	  }
	}
	else {
	  std::array<double, 2> bmu {(argmax % side)*coef, (argmax / side)*coef};
	  double d2 = bmu[0] - current2[0];
	  double dy = bmu[1] - current2[1];
	  d2 = d2*d2 + dy*dy;
	  std::array<double, 2> tmp;
	  if(d2 <= delta2)
	    tmp = bmu;
	  else {
	    double d_ = delta/std::sqrt(d2);
	    tmp[0] = current2[0] + (bmu[0] - current2[0])*d_;
	    tmp[1] = current2[1] + (bmu[1] - current2[1])*d_;
	  }

	  if(std::max(std::fabs(static_cast<data::d2::Pos&>(data).xy[0] - tmp[0]),
		      std::fabs(static_cast<data::d2::Pos&>(data).xy[1] - tmp[1])) > epsilon) {
	    static_cast<data::d2::Pos&>(data).xy = tmp;
	    return !expired();
	  }
	  else {
	    static_cast<data::d2::Pos&>(data).xy = tmp;
	    return false;
	  }
	}
      }
    };
    
    //////////////////////
    //                  //
    // TowardConvArgmax // 
    //                  //
    //////////////////////

    class TowardConvArgmax : public ArgmaxBase {
    private:
      double sigma;
      double delta;
      double delta2;

      std::shared_ptr<fftconv::Convolution> conv_ptr = nullptr;
      std::vector<double> data_content;
      type::ref map_type = nullptr;
      double current1;
      std::array<double, 2> current2;

      void debug(double bmu) {

	// See the python-debug-tools directory for plotting the generated fils.

	// Modify this code for your specific debug...
	std::set<cxsom::symbol::Variable> variables = {
						       {"rlx", "X/BMU"},
						       {"rlx", "Y/BMU"}
	};
	if(variables.find(result.who.variable) == variables.end())
	  return;
	std::ostringstream filename;
	std::string varname = result.who.variable.name;
	for(auto& c : varname)
	  if(c == '/') c = '-';
	filename << "tca-" << std::setfill('0') << std::setw(6) << ticker->t()
		 << '-' << result.who.variable.timeline << '-' << varname << '-'
		 << std::setfill('0') << std::setw(6) << result.who.at
		 << '-' << std::setfill('0') << std::setw(6) << tick_time() << ".data";
	std::ofstream file(filename.str().c_str());
	file << size << std::endl
	     << current1 << std::endl
	     << bmu << std::endl;
	const double* it  = conv_ptr->ws.dst;
	const double* end = it + size;
	while(it != end) file << *(it++) << ' ';
	file << std::endl;
	it  = std::data(conv_ptr->data);
	end = it + size;
	while(it != end) file << *(it++) << ' ';
	std::cout << "\"" << filename.str() << "\" generated." << std::endl;
      }
      

      void read_current(const data::Base& data) {
	if(data.type->is_Pos1D())
	  current1 = static_cast<const data::d1::Pos&>(data).x;
	else
	  current2 = static_cast<const data::d2::Pos&>(data).xy;
      }
      
      void read_map(const data::Base& data) {
	auto& vec = static_cast<const data::Map&>(data).content;
	data_content.resize(vec.size());
	std::copy(vec.begin(), vec.end(), data_content.begin());

	if(!conv_ptr) {
	  map_type = data.type;
	  side = static_cast<const type::Map*>(map_type.get())->side;
	  size = static_cast<const type::Map*>(map_type.get())->size;
	  coef = 1/(side - 1.0);
	  unsigned int h = 1;
	  if(map_type->is_Map2D()) h = (unsigned int)side;
	  conv_ptr = std::make_shared<fftconv::Convolution>((unsigned int)side, h, sigma*side,
							    data_content,
							    fftconv::KernelType::Gaussian,
							    fftconv::PaddingType::Constant);
	}
	conv_ptr->convolve();
      }
      
    public:

      TowardConvArgmax(data::Center& center,
		       const update::arg& res,
		       const std::vector<update::arg>& args,
		       const std::map<std::string, std::string>& params,
		       std::mt19937::result_type seed)
	: ArgmaxBase(center, res, "toward-conv-argmax", args, params, seed), sigma(.1), delta(.05) {
	if(auto it = params.find("sigma");   it != params.end()) sigma   = std::stod(it->second);
	if(auto it = params.find("delta");   it != params.end()) delta   = std::stod(it->second);
	delta2 = delta*delta;
      }
      
    protected:
  
      virtual void on_read_out_arg(const symbol::Instance&, unsigned int arg_num, const data::Base& data) override {
	switch(arg_num) {
	case 0 : read_map(data);     break;
	case 1 : read_current(data); break;
	}
      }
      
      virtual void on_read_in_arg(const symbol::Instance&, unsigned int arg_num, const data::Base& data) override {
	switch(arg_num) {
	case 0 : read_map(data);     break;
	case 1 : read_current(data); break;
	}
      }
      
      virtual bool on_write_result(data::Base& data) override {
	tick();
	auto argmax = find_argmax(conv_ptr->ws.dst, size);
	if(data.type->is_Pos1D()) {
	  double bmu = argmax*coef;
	  double d = std::fabs(bmu - current1);
	  double tmp;
	  if(d <= delta)          tmp = bmu;
	  else if(current1 > bmu) tmp = current1 - delta;
	  else                    tmp = current1 + delta;
#ifdef cxsomDEBUG_TOWARD_CONV_ARGMAX
	  debug(bmu);
#endif
	  if(std::fabs(static_cast<data::d1::Pos&>(data).x - tmp) > epsilon) {
	    static_cast<data::d1::Pos&>(data).x = tmp;
	    return !expired();
	  }
	  else {
	    static_cast<data::d1::Pos&>(data).x = tmp;
	    return false;
	  }
	}
	else {
	  std::array<double, 2> bmu {(argmax % side)*coef, (argmax / side)*coef};
	  double d2 = bmu[0] - current2[0];
	  double dy = bmu[1] - current2[1];
	  d2 = d2*d2 + dy*dy;
	  std::array<double, 2> tmp;
	  if(d2 <= delta2)
	    tmp = bmu;
	  else {
	    double d_ = delta/std::sqrt(d2);
	    tmp[0] = current2[0] + (bmu[0] - current2[0])*d_;
	    tmp[1] = current2[1] + (bmu[1] - current2[1])*d_;
	  }

	  if(std::max(std::fabs(static_cast<data::d2::Pos&>(data).xy[0] - tmp[0]),
		      std::fabs(static_cast<data::d2::Pos&>(data).xy[1] - tmp[1])) > epsilon) {
	    static_cast<data::d2::Pos&>(data).xy = tmp;
	    return !expired();
	  }
	  else {
	    static_cast<data::d2::Pos&>(data).xy = tmp;
	    return false;
	  }
	}
      }
    };

    
    ////////////////////
    //                //
    // Update factory //
    //                //
    ////////////////////

    /**
     * This class provides update instances from the op name.
     */
    struct UpdateFactory {
    public:
      using make_update_type = std::function<update::ref (const update::arg&,
							  const std::vector<update::arg>&,
							  const std::map<std::string, std::string>&,
							  std::mt19937::result_type seed)>;
    private:
      std::map<Operation, make_update_type> factory;
      
    public:

      void operator+=(const std::pair<Operation, make_update_type>& make_updt) {
	if(auto it = factory.find(std::get<0>(make_updt)); it == factory.end())
	  factory[std::get<0>(make_updt)] = std::get<1>(make_updt);
	else
	  throw error::already_existing_update(std::get<0>(make_updt));
      }

      update::ref operator()(data::Center& center,
			     Operation op,
			     const update::arg& res,
			     const std::vector<update::arg>& args,
			     const std::map<std::string, std::string>& params,
			     std::mt19937::result_type seed) const {
	if(auto it = factory.find(op); it != factory.end())
	  return stg::get<1>(*it)(center, res, args, params, seed);
	else
	  throw error::not_existing_update(op);
      }
    };

    void fill(UpdateFactory& factory) {
      factory += {"copy"              , std::bind(make_update_deterministic<Copy>         , _1, _2, _3, _4, _5)};
      factory += {"average"           , std::bind(make_update_deterministic<Average>      , _1, _2, _3, _4, _5)};
      factory += {"random"            , std::bind(make_update_random<Random>              , _1, _2, _3, _4, _5)};
      factory += {"converge"          , std::bind(make_update_deterministic<Converge>     , _1, _2, _3, _4, _5)};
      factory += {"clear"             , std::bind(make_update_deterministic<Clear>        , _1, _2, _3, _4, _5)};
      factory += {"merge"             , std::bind(make_update_deterministic<Merge>        , _1, _2, _3, _4, _5)};
      factory += {"match-triangle"    , std::bind(make_update_deterministic<MatchTriangle>, _1, _2, _3, _4, _5)};
      factory += {"match-gaussian"    , std::bind(make_update_deterministic<MatchGaussian>, _1, _2, _3, _4, _5)};
      factory += {"learn-triangle"    , std::bind(make_update_deterministic<LearnTriangle>, _1, _2, _3, _4, _5)};
      factory += {"learn-gaussian"    , std::bind(make_update_deterministic<LearnGaussian>, _1, _2, _3, _4, _5)};
      factory += {"argmax"            , std::bind(make_update_random<Argmax>              , _1, _2, _3, _4, _5)};
      factory += {"conv-argmax"       , std::bind(make_update_random<ConvArgmax>          , _1, _2, _3, _4, _5)};
      factory += {"toward-argmax"     , std::bind(make_update_random<TowardArgmax>        , _1, _2, _3, _4, _5)};
      factory += {"toward-conv-argmax", std::bind(make_update_random<TowardConvArgmax>    , _1, _2, _3, _4, _5)};
    }


    ///////////////////
    //               //
    // Type checking //
    //               //
    ///////////////////

    inline void check_types_copy(type::ref res, const std::vector<type::ref>& args) {
      std::ostringstream ostr;
      if(args.size() == 1) {
	if(*res == *(args[0]))
	  return;
	ostr << "Checking types for Copy : Result value has type " << res->name()
	     << " while argument has type " << args[0]->name() << '.';
      }
      else
	ostr << "Checking types for Copy : Exactly one argument is expected (got " << args.size() << ").";
      throw error::bad_typing(ostr.str());
    }
    
    inline void check_types_average(type::ref res, const std::vector<type::ref>& args) {
      if(!(res->is_Map("Scalar"))) {
	std::ostringstream ostr;
	ostr << "cxsom::jobs::Average : Type " << res->name() << " is not accepted for result.";
	throw error::bad_typing(ostr.str());
      }

      for(auto it = args.begin(); it != args.end(); ++it)
	if(*(*it) != *res) {
	  std::ostringstream ostr;
	  ostr << "Checking types for Average : Result has type " << res->name() << " while argument #"
	       << std::distance(args.begin(), it) << " has type " << (*it)->name() << '.';
	  throw error::bad_typing(ostr.str());
	}
    }

    inline void check_types_random(type::ref, const std::vector<type::ref>& args) {
      std::ostringstream ostr;
      if(args.size() == 0)
	return;
      else
	ostr << "Checking types for Random : No argument expected (got " << args.size() << ").";
      throw error::bad_typing(ostr.str());
    }
    
    inline void check_types_converge(type::ref res, const std::vector<type::ref>&) {
      std::ostringstream ostr;
      if(res->is_Scalar()) 
	return;
      else
	ostr << "Checking types for Converge : Expecting scalar result (got " << res->name() << ").";
      throw error::bad_typing(ostr.str());
    }
    
    inline void check_types_clear(type::ref, const std::vector<type::ref>& args) {
      std::ostringstream ostr;
      if(args.size() == 0)
	return;
      else
	ostr << "Checking types for Clear : No argument expected (got " << args.size() << ").";
      throw error::bad_typing(ostr.str());
    }
    
    inline void check_types_merge(type::ref res, const std::vector<type::ref>& args) {
      std::ostringstream ostr;
      if(args.size() == 2) {
	if(res->is_Map("Scalar")) {
	  if((*res == *(args[0])) && (*res == *(args[1])))
	    return;
	  else
	    ostr << "Checking types for Merge: Both arguments are not similar to result : result is "
		 << res->name() << " while args are " << args[0]->name() << "and"
		 << args[1]->name() << '.';
	}
	else
	  ostr << "Checking types for Merge: Result has invalid type " << res->name() << '.';
      }
      else
	ostr << "Checking types for Merge : Exactly 2 arguments are expected (got " << args.size() << ").";
      throw error::bad_typing(ostr.str());
    }
    
    inline void check_types_match(type::ref res, const std::vector<type::ref>& args) {
      std::ostringstream ostr;
      if(args.size() == 2) {
	if(res->is_Map("Scalar")) {
	  if(args[1]->is_Map(args[0]->name())) {
	    if(((res->is_Map1D() && args[1]->is_Map1D())
		|| (res->is_Map2D() && args[1]->is_Map2D()))
	       && (static_cast<const type::Map*>(res.get())->side == static_cast<const type::Map*>(args[1].get())->side)) {
	      if((   args[0]->is_Scalar()    &&  args[1]->is_Map("Scalar"))
		 || (args[0]->is_Pos1D()     &&  args[1]->is_Map("Pos1D"))
		 || (args[0]->is_Pos2D()     &&  args[1]->is_Map("Pos2D"))
		 || (args[0]->is_Array() > 0 && (args[0]->is_Array() == args[1]->is_MapOfArray())))
		return;
	      else
		ostr << "Checking types for Match: Input (i.e. " << args[0]->name() << ") and the content of the weight map (i.e. " << args[1]->name() << ") are invalid.";
		
	    }
	    else
	      ostr << "Checking types for Match: Result (i.e. " << res->name() << ") and weights (i.e. "
		   << args[1]->name() << ") are not the same kind of map." << std::endl;
	  }
	  else
	    ostr << "Checking types for Match: Input type is " << args[0]->name()
		 << " while weights is " << args[1]->name() << '.';
	}
	else
	  ostr << "Checking types for Match: Result has invalid type " << res->name() << '.';
      }
      else
	ostr << "Checking types for Match : Exactly 2 arguments are expected (got " << args.size() << ").";
      throw error::bad_typing(ostr.str());
    }
    
    inline void check_types_learn(type::ref res, const std::vector<type::ref>& args) {
      std::ostringstream ostr;
      if(args.size() == 3) {
	if(*res == *(args[1])) {
	  if(res->is_Map()) {
	    if((   args[0]->is_Scalar()    &&  args[1]->is_Map("Scalar"))
	       || (args[0]->is_Pos1D()     &&  args[1]->is_Map("Pos1D"))
	       || (args[0]->is_Pos2D()     &&  args[1]->is_Map("Pos2D"))
	       || (args[0]->is_Array() > 0 && (args[0]->is_Array() == args[1]->is_MapOfArray()))) {
	      if((   args[1]->is_Map1D() && args[2]->is_Pos1D())
		 || (args[1]->is_Map2D() && args[2]->is_Pos2D()))
		return;
	      else
		ostr << "Checking types for Learn: Third argument (BMU, i.e. " << args[2]->name()
		     << ") and weight maps (i.e " << res->name() << ") do not match.";
	    }
	    else
	      ostr << "Checking types for Learn: Input (i.e. " << args[0]->name() << ") and the content of the weight map (i.e. " << args[1]->name() << ") are invalid.";
	  }
	  else
	    ostr << "Checking types for Learn : Second argument and result should be maps (found type " << res->name() << ").";
	}
	else
	  ostr << "Checking types for Learn : Second argument and result should have the same type. Here, result has type "
	       << res->name() << " while second argument has type "
	       << args[1]->name() << '.';
      }
      else
	ostr << "Checking types for Learn : Exactly 3 arguments are expected (got " << args.size() << ").";
      throw error::bad_typing(ostr.str());
    }
    
    inline void check_types_argmax(type::ref res, const std::vector<type::ref>& args) {
      std::ostringstream ostr;
      if(args.size() == 1) {
	if(args[0]->is_Map1D("Scalar") || args[0]->is_Map2D("Scalar")) {
	  if((   res->is_Pos1D() && args[0]->is_Map1D("Scalar"))
	     || (res->is_Pos2D() && args[0]->is_Map2D("Scalar")))
	    return;
	  else
	    ostr << "Checking types for ConvArgmax : Result must fit the map dimension. Here, result has type "
		 << res->name() << " while argument is " << args[0]->name() << '.';
	}
	else
	  ostr << "Checking types for ConvArgmax : Argument have to be a map of scalar (" << args[0]->name() << " found).";
      }
      else
	ostr << "Checking types for ConvArgmax : Exactly 1 arguments is expected (got " << args.size() << ").";
      throw error::bad_typing(ostr.str());
    }
    
    inline void check_types_toward_argmax(type::ref res, const std::vector<type::ref>& args) {
      std::ostringstream ostr;
      if(args.size() == 2) {
	if(args[0]->is_Map1D("Scalar") || args[0]->is_Map2D("Scalar")) {
	  if((   res->is_Pos1D() && args[0]->is_Map1D("Scalar"))
	     || (res->is_Pos2D() && args[0]->is_Map2D("Scalar"))) {
	    if(*res == *(args[1]))
	      return;
	    else
	      ostr << "Checking types for TowardArgmax : Result and second argument must have the same type. Here, "
		   << "result has type " << res->name() << " while second argument is " << args[1]->name() << '.';
	  }
	  else
	    ostr << "Checking types for TowardArgmax : Result must fit the map dimension. Here, result has type "
		 << res->name() << " while first argument is " << args[0]->name() << '.';
	}
	else
	  ostr << "Checking types for TowardArgmax : Argument have to be a map of scalar (" << args[0]->name() << " found).";
      }
      else
	ostr << "Checking types for TowardArgmax : Exactly 2 arguments is expected (got " << args.size() << ").";
      throw error::bad_typing(ostr.str());
    }
    
    inline void check_types_toward_conv_argmax(type::ref res, const std::vector<type::ref>& args) {
      std::ostringstream ostr;
      if(args.size() == 2) {
	if(args[0]->is_Map1D("Scalar") || args[0]->is_Map2D("Scalar")) {
	  if((   res->is_Pos1D() && args[0]->is_Map1D("Scalar"))
	     || (res->is_Pos2D() && args[0]->is_Map2D("Scalar"))) {
	    if(*res == *(args[1]))
	      return;
	    else
	      ostr << "Checking types for TowardConvArgmax : Result and second argument must have the same type. Here, "
		   << "result has type " << res->name() << " while second argument is " << args[1]->name() << '.';
	  }
	  else
	    ostr << "Checking types for TowardConvArgmax : Result must fit the map dimension. Here, result has type "
		 << res->name() << " while first argument is " << args[0]->name() << '.';
	}
	else
	  ostr << "Checking types for TowardConvArgmax : Argument have to be a map of scalar (" << args[0]->name() << " found).";
      }
      else
	ostr << "Checking types for TowardConvArgmax : Exactly 2 arguments is expected (got " << args.size() << ").";
      throw error::bad_typing(ostr.str());
    }
      
    inline void check_types(Operation op, type::ref res, const std::vector<type::ref>& args) {
      switch(op) {
      case Operation::Copy             : check_types_copy               (res, args); break;
      case Operation::Average          : check_types_average            (res, args); break;
      case Operation::Random           : check_types_random             (res, args); break;
      case Operation::Converge         : check_types_converge           (res, args); break;
      case Operation::Clear            : check_types_clear              (res, args); break;
      case Operation::Merge            : check_types_merge              (res, args); break;
      case Operation::MatchTriangle    : check_types_match              (res, args); break;
      case Operation::MatchGaussian    : check_types_match              (res, args); break;
      case Operation::LearnTriangle    : check_types_learn              (res, args); break;
      case Operation::LearnGaussian    : check_types_learn              (res, args); break;
      case Operation::Argmax           : check_types_argmax             (res, args); break;
      case Operation::ConvArgmax       : check_types_argmax             (res, args); break;
      case Operation::TowardArgmax     : check_types_toward_argmax      (res, args); break;
      case Operation::TowardConvArgmax : check_types_toward_conv_argmax (res, args); break;
      default:
	throw error::bad_operation("cxsom::job::check_types(op, ...) : report bug.");
	break;
      }
    }
  }
}
