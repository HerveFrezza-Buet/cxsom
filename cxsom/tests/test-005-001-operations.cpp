#include <cxsom-rules.hpp>
#include <cxsom-server.hpp>

// This is used in case of some macros are set. You can remove this
// lines otherwise... or keep it.
cxsom::Tick* cxsom::ticker = new cxsom::Tick();
cxsom::Log*  cxsom::logger = new cxsom::Log();

#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <random>

#include <filesystem>
namespace fs = std::filesystem;

// This is for further convenience
using namespace cxsom::rules;

// This is a mandatory declaration, it implements some global variable. 
context* cxsom::rules::ctx = nullptr;

#define WALLTIME      100
#define CACHE_SIZE     10
#define FILE_SIZE    1000
#define KEPT_OPENED  true


namespace test {
  class All;

  // ########
  // #      # 
  // # Base # 
  // #      # 
  // ########
  
  class Base {
    
  protected:

    friend class All;
    
    fs::path                  root_dir;
    std::string               name;
    std::string               op_name;
    std::string               res;
    std::vector<std::string>  args;
    
    virtual void make_args()        const = 0;
    virtual void send_computation() const = 0;
    // returs true in case of failure.
    virtual bool test_result()      const = 0;


  protected:

    
  public:
    
    Base()                       = delete;
    Base(const Base&)            = delete;
    Base& operator=(const Base&) = delete;
    
    Base(const fs::path& root_dir,
	 const std::string& name, const std::string& op_name,
	 const std::string& res,
	 const std::vector<std::string>& args)
      : root_dir(root_dir), name(name), op_name(op_name), res(res), args(args) {}

    operator std::string () const {
      std::ostringstream ostr;
      ostr << "\e[1m\e[95m" << std::setw(30) <<  std::left << name
	   << "\e[0m: " << res << " = " << op_name << '(';
      auto it = args.begin();
      if(it != args.end())
	ostr << *(it++);
      while(it != args.end())
	ostr << ", " << *(it++);
      ostr << ')';
      return ostr.str();
    }
    
    bool compute_error_status() {
      if(test_result()) {
	std::cout << "[\e[1m\e[31mFAILED\e[0m] " << std::string(*this) << std::endl;
	return true;
      }
      std::cout << "[  \e[1m\e[32mOK\e[0m  ] " << std::string(*this) << std::endl;
      return false;
    }
  };

  // #########
  // #       # 
  // # Clear # 
  // #       # 
  // #########
  
  class Clear : public Base {
  protected:
    double value;
  
  public:

    Clear(const fs::path& root_dir,
	  const std::string& name,
	  const std::string& res,
	  double value) : Base(root_dir, name, "clear", res, {}), value(value) {}

    virtual void make_args() const override {}
  
    virtual void send_computation() const override {
      timeline t(name);

      kwd::type("res", res, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
      kwd::at("res", 0) << fx::clear() | kwd::use("value", value);
    }
  
    virtual bool test_result() const override {
      bool error_status = false;

      try {
	cxsom::data::Variable v(root_dir, {name, "res"}, nullptr, std::nullopt, std::nullopt, true);
	v[0]->get([&error_status, this](auto status, auto, auto& data) {
		    if(status != cxsom::data::Availability::Ready) {
		      error_status = true;
		      return;
		    }
		    
		    if(data.type->name() != this->res) {
		      error_status = true;
		      return;
		    }
		    
		    if(data.type->is_Scalar()) {
		      error_status = (static_cast<const cxsom::data::Scalar&>(data).value != .5);
		      return;
		    }
		    else if(data.type->is_Pos1D()) {
		      error_status = (static_cast<const cxsom::data::d1::Pos&>(data).x != .5);
		      return;
		    }
		    else if(data.type->is_Pos2D()) {
		      auto& xy = static_cast<const cxsom::data::d2::Pos&>(data).xy;
		      error_status = (xy[0] != .5 || xy[1] != .5);
		      return;
		    }
		    else if(auto dim = data.type->is_Array(); dim > 0) {
		      auto& content = static_cast<const cxsom::data::Array&>(data).content;
		      for(auto v : content)
			if(v != .5) {
			  error_status = true;
			  break;
			}
		      return;
		    }
		    else if(data.type->is_Map()) {
		      auto& content = static_cast<const cxsom::data::Map&>(data).content;
		      for(auto v : content)
			if(v != .5) {
			  error_status = true;
			  break;
			}
		      return;
		    }

		    error_status = true;
		  });
      } catch(std::exception& e) {
	std::cout << "Exception : " << e.what() << std::endl;
	error_status = true;
      }

      return error_status;
    }
  };

  
  // ########
  // #      # 
  // # Copy # 
  // #      # 
  // ########
  
  class Copy : public Base {
  protected:
  
  public:

    Copy(const fs::path& root_dir,
	 const std::string& name,
	 const std::string& res) : Base(root_dir, name, "copy", res, {res}) {}

    virtual void make_args() const override {}
  
    virtual void send_computation() const override {
      {
	timeline t(name + "_args");
	kwd::type("x", args[0], CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::at("x", 0) << fx::random();
      }

      {
	timeline t(name);
	kwd::type("res", res, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::at("res", 0) << fx::copy(kwd::at(kwd::var(name + "_args", "x"), 0));
      }
    }
  
    virtual bool test_result() const override {
      bool error_status = false;

      try {
	cxsom::data::Variable v(root_dir, {name, "res"}, nullptr, std::nullopt, std::nullopt, true);
	cxsom::data::Variable x(root_dir, {name + "_args", "x"}, nullptr, std::nullopt, std::nullopt, true);
	std::vector<char> x_buf;

	x[0]->get([&x_buf](auto, auto, auto& data) {
		    x_buf.resize(data.type->byte_length());
		    data.write(std::data(x_buf));
		  });
	v[0]->get([&error_status, &x_buf, this](auto status, auto, auto& data) {
		    if(status != cxsom::data::Availability::Ready) {
		      error_status = true;
		      return;
		    }
		    
		    if(data.type->name() != this->res) {
		      error_status = true;
		      return;
		    }

		    std::vector<char> res_buf;
		    res_buf.resize(data.type->byte_length());
		    data.write(std::data(res_buf));

		    if(res_buf.size() != x_buf.size()) {
		      error_status = true;
		      return;
		    }

		    error_status = !std::equal(x_buf.begin(), x_buf.end(), res_buf.begin());
		  });
      } catch(std::exception& e) {
	std::cout << "Exception : " << e.what() << std::endl;
	error_status = true;
      }
      
      return error_status;
    }
  };

  // ###########
  // #         # 
  // # Average # 
  // #         # 
  // ###########
  
  class Average : public Base {
  protected:
  
  public:

    Average(const fs::path& root_dir,
	    const std::string& name,
	    const std::string& res) : Base(root_dir, name, "average", res, {res, res, res}) {}

    virtual void make_args() const override {}
  
    virtual void send_computation() const override {
      {
	timeline t(name + "_args");
	kwd::type("x", args[0], CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::at("x", 0) << fx::random();
	kwd::type("y", args[1], CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::at("y", 0) << fx::random();
	kwd::type("z", args[2], CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::at("z", 0) << fx::random();
      }

      {
	timeline t(name);
	kwd::type("res", res, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::at("res", 0) << fx::average({kwd::at(kwd::var(name + "_args", "x"), 0),
					  kwd::at(kwd::var(name + "_args", "y"), 0),
					  kwd::at(kwd::var(name + "_args", "z"), 0)});
      }
    }
  
    virtual bool test_result() const override {
      bool error_status = false;

      try {
	cxsom::data::Variable v(root_dir, {name, "res"}, nullptr, std::nullopt, std::nullopt, true);
	cxsom::data::Variable x(root_dir, {name + "_args", "x"}, nullptr, std::nullopt, std::nullopt, true);
	cxsom::data::Variable y(root_dir, {name + "_args", "y"}, nullptr, std::nullopt, std::nullopt, true);
	cxsom::data::Variable z(root_dir, {name + "_args", "z"}, nullptr, std::nullopt, std::nullopt, true);
	std::vector<char> x_buf;
	std::vector<char> y_buf;
	std::vector<char> z_buf;

	x[0]->get([&x_buf](auto, auto, auto& data) {
		    x_buf.resize(data.type->byte_length());
		    data.write(std::data(x_buf));
		  });
	y[0]->get([&y_buf](auto, auto, auto& data) {
		    y_buf.resize(data.type->byte_length());
		    data.write(std::data(y_buf));
		  });
	z[0]->get([&z_buf](auto, auto, auto& data) {
		    z_buf.resize(data.type->byte_length());
		    data.write(std::data(z_buf));
		  });
	v[0]->get([&error_status, &x_buf, &y_buf, &z_buf, this](auto status, auto, auto& data) {
		    if(status != cxsom::data::Availability::Ready) {
		      error_status = true;
		      return;
		    }
		    
		    if(data.type->name() != this->res) {
		      error_status = true;
		      return;
		    }

		    if(data.type->is_Map("Scalar")) {
		      std::vector<char> res_buf;
		      res_buf.resize(data.type->byte_length());
		      data.write(std::data(res_buf));

		      if(res_buf.size() != x_buf.size()
			 || res_buf.size() != y_buf.size()
			 || res_buf.size() != z_buf.size()) {
			error_status = true;
			return;
		      }

		      double* avg     = reinterpret_cast<double*>(std::data(res_buf));
		      double* x       = reinterpret_cast<double*>(std::data(x_buf));
		      double* y       = reinterpret_cast<double*>(std::data(y_buf));
		      double* z       = reinterpret_cast<double*>(std::data(z_buf));
		      double* avg_end = avg + static_cast<const cxsom::type::Map*>(data.type.get())->size;

		      while((!error_status) && (avg != avg_end))
			error_status = (*(avg++) != (*(x++) + *(y++) + *(z++))*(1/3.0));
		      
		      return;
		    }
		    error_status = true;
		    
		  });
      } catch(std::exception& e) {
	std::cout << "Exception : " << e.what() << std::endl;
	error_status = true;
      }
      
      return error_status;
    }
  };



  // #########
  // #       # 
  // # Merge # 
  // #       # 
  // #########
  
  class Merge : public Base {
  protected:

    double beta;
    
  public:

    Merge(const fs::path& root_dir,
	  const std::string& name,
	  const std::string& res,
	  double beta) : Base(root_dir, name, "merge", res, {res, res}), beta(beta) {}

    virtual void make_args() const override {}
  
    virtual void send_computation() const override {
      {
	timeline t(name + "_args");
	kwd::type("x", args[0], CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::at("x", 0) << fx::random();
	kwd::type("y", args[1], CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::at("y", 0) << fx::random();
      }

      {
	timeline t(name);
	kwd::type("res", res, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::at("res", 0) << fx::merge(kwd::at(kwd::var(name + "_args", "x"), 0),
				       kwd::at(kwd::var(name + "_args", "y"), 0)) | kwd::use("beta", beta);
      }
    }
  
    virtual bool test_result() const override {
      bool error_status = false;

      try {
	cxsom::data::Variable v(root_dir, {name, "res"}, nullptr, std::nullopt, std::nullopt, true);
	cxsom::data::Variable x(root_dir, {name + "_args", "x"}, nullptr, std::nullopt, std::nullopt, true);
	cxsom::data::Variable y(root_dir, {name + "_args", "y"}, nullptr, std::nullopt, std::nullopt, true);
	std::vector<char> x_buf;
	std::vector<char> y_buf;

	x[0]->get([&x_buf](auto, auto, auto& data) {
		    x_buf.resize(data.type->byte_length());
		    data.write(std::data(x_buf));
		  });
	y[0]->get([&y_buf](auto, auto, auto& data) {
		    y_buf.resize(data.type->byte_length());
		    data.write(std::data(y_buf));
		  });
	v[0]->get([&error_status, &x_buf, &y_buf, this](auto status, auto, auto& data) {
		    if(status != cxsom::data::Availability::Ready) {
		      error_status = true;
		      return;
		    }
		    
		    if(data.type->name() != this->res) {
		      error_status = true;
		      return;
		    }

		    if(data.type->is_Map("Scalar")) {
		      std::vector<char> res_buf;
		      res_buf.resize(data.type->byte_length());
		      data.write(std::data(res_buf));

		      if(res_buf.size() != x_buf.size()
			 || res_buf.size() != y_buf.size()) {
			error_status = true;
			return;
		      }

		      double* avg     = reinterpret_cast<double*>(std::data(res_buf));
		      double* x       = reinterpret_cast<double*>(std::data(x_buf));
		      double* y       = reinterpret_cast<double*>(std::data(y_buf));
		      double* avg_end = avg + static_cast<const cxsom::type::Map*>(data.type.get())->size;

		      while((!error_status) && (avg != avg_end)) {
			error_status = (*(avg++) != std::sqrt(*x * (beta * *x + (1 - beta) * *(y++))));
			++x;
		      }
		      
		      return;
		    }
		    
		    error_status = true;
		    
		  });
      } catch(std::exception& e) {
	std::cout << "Exception : " << e.what() << std::endl;
	error_status = true;
      }
      
      return error_status;
    }
  };

  inline std::string map_of(unsigned int dim, unsigned int side, const std::string& content) {
    std::ostringstream ostr;
    ostr << "Map" << dim << "D<" << content << ">=" << side;
    return ostr.str();
  }
  
  inline std::string pos_of(unsigned int dim) {
    std::ostringstream ostr;
    ostr << "Pos" << dim << 'D';
    return ostr.str();
  }
  
  // #################
  // #               # 
  // # MatchTriangle # 
  // #               # 
  // #################
  
  class MatchTriangle : public Base {
  protected:

    double r;
    double tol;
    
  public:

    MatchTriangle(const fs::path& root_dir,
		  const std::string& name,
		  unsigned int mapdim,
		  const std::string& content,
		  double r, double tol)
      : Base(root_dir, name, "match_triangle", map_of(mapdim, 100, "Scalar"), {content, map_of(mapdim, 100, content)}), r(r), tol(tol) {}

    virtual void make_args() const override {
      
      fs::create_directories(root_dir / (name + "_args"));
      
      {
	cxsom::data::File file(root_dir, {name + "_args", "wgt"});
	file.realize(cxsom::type::make(args[1]), CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
      }

      {
	cxsom::data::Variable v(root_dir, {name + "_args", "wgt"}, nullptr, std::nullopt, std::nullopt, true);
	v[0]->set([](auto& status, auto&, auto& data) {
		    status = cxsom::data::Availability::Ready;
		    unsigned int side = static_cast<const cxsom::type::Map*>(data.type.get())->side;
		    double coef = 1/(side - 1.0);
		    auto& content = static_cast<cxsom::data::Map&>(data).content;
		    if(data.type->is_Map1D("Scalar") || data.type->is_Map1D("Pos1D")) {
		      unsigned int i = 0;
		      for(auto& e : content) e = coef * (i++);
		    }
		    else if(data.type->is_Map1D("Pos2D")) {
		      unsigned int i = 0;
		      auto it = content.begin();
		      while(it != content.end()) {
			*(it++) = coef * i;
			*(it++) = coef * (i++);
		      }
		    }
		    else if(auto dim = data.type->is_Map1DOfArray(); dim > 0) {
		      double coef = 1/(side - 1.0);
		      unsigned int i = 0;
		      auto it = content.begin();
		      while(it != content.end()) {
			auto dimit_end = it + dim;
			while(it != dimit_end) *(it++) = coef * i;
			++i;
		      }
		    }
		    else if(data.type->is_Map2D("Scalar") || data.type->is_Map2D("Pos1D")) {
		      auto it = content.begin();
		      for(unsigned int i = 0; i < side; ++i)
			for(unsigned int j = 0; j<side; ++j)
			  *(it++) = coef * j;
		    }
		    else if(data.type->is_Map2D("Pos2D")) {
		      auto it = content.begin();
		      for(unsigned int i = 0; i < side; ++i)
			for(unsigned int j = 0; j < side; ++j) {
			  *(it++) = coef * j;
			  *(it++) = coef * j;
			}
		    }
		    else if(auto dim = data.type->is_Map2DOfArray(); dim > 0) {
		      auto it = content.begin();
		      for(unsigned int i = 0; i < side; ++i)
			for(unsigned int j = 0; j < side; ++j) {
			  auto dimit_end = it + dim;
			  while(it != dimit_end)
			    *(it++) = coef * j;
			}
		    }
		    else
		      status = cxsom::data::Availability::Busy;
		  });
      }

      
    }
  
    virtual void send_computation() const override {
      {
	timeline t(name + "_args");
	kwd::type("xi", args[0], CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::at("xi", 0) << fx::clear() | kwd::use("value", .5);
      }
      {
	timeline t(name);
	kwd::type("res", res, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::at("res", 0) << fx::match_triangle(kwd::at(kwd::var(name + "_args", "xi"), 0),
						kwd::at(kwd::var(name + "_args", "wgt"), 0)) | kwd::use("r", r);
      }
	
    }
  
    virtual bool test_result() const override {
      bool error_status = false;

      try {
	cxsom::data::Variable v(root_dir, {name, "res"}, nullptr, std::nullopt, std::nullopt, true);
	auto input_type = cxsom::type::make(args[0]);
	double dim_coef = 1;
	if(input_type->is_Pos2D()) dim_coef = std::sqrt(2);
	else if(auto dim = input_type->is_Array(); dim > 0) dim_coef = sqrt(dim);
	v[0]->get([&error_status, this, dim_coef](auto status, auto, auto& data) {
		    if(status != cxsom::data::Availability::Ready) {
		      error_status = true;
		      return;
		    }
		    
		    if(data.type->name() != this->res) {
		      error_status = true;
		      return;
		    }

		    if(data.type->is_Map("Scalar")) {
		      auto& content = static_cast<const cxsom::data::Map&>(data).content;
		      auto     side = static_cast<const cxsom::type::Map*>(data.type.get())->side;

		      auto check_pos = [a = 1/r, coef = 1/(side - 1.0), dim_coef](unsigned int i) {return cxsom::match::triangle(a * dim_coef, i * coef, .5);};
		      
		      if(data.type->is_Map1D()) {
			unsigned int i = 0;
			for(auto e : content)
			  if(fabs(check_pos(i++) - e) > tol) {
			    error_status = true;
			    return;
			  }
			return;
		      }
		      else if(data.type->is_Map2D()) {
			auto it = content.begin();
			for(unsigned int i = 0; i < side; ++i)
			  for(unsigned int j = 0; j < side; ++j)
			    if(fabs(check_pos(j) - *(it++)) > tol) {
			      error_status = true;
			      return;
			    }
			return;
		      }
		      else {
			std::cout << data.type->name() << std::endl;
			error_status = true;
		      }
		      return;
		    }
		    
		    error_status = true;
		    
		  });
      } catch(std::exception& e) {
	std::cout << "Exception : " << e.what() << std::endl;
	error_status = true;
      }
      
      return error_status;
    }
  };
  
  // #################
  // #               # 
  // # MatchGaussian # 
  // #               # 
  // #################
  
  class MatchGaussian : public Base {
  protected:

    double sigma;
    double tol;
    
  public:

    MatchGaussian(const fs::path& root_dir,
		  const std::string& name,
		  unsigned int mapdim,
		  const std::string& content,
		  double sigma, double tol)
      : Base(root_dir, name, "match_gaussian", map_of(mapdim, 100, "Scalar"), {content, map_of(mapdim, 100, content)}), sigma(sigma), tol(tol) {}

    virtual void make_args() const override {
      
      fs::create_directories(root_dir / (name + "_args"));
      
      {
	cxsom::data::File file(root_dir, {name + "_args", "wgt"});
	file.realize(cxsom::type::make(args[1]), CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
      }

      {
	cxsom::data::Variable v(root_dir, {name + "_args", "wgt"}, nullptr, std::nullopt, std::nullopt, true);
	v[0]->set([](auto& status, auto&, auto& data) {
		    status = cxsom::data::Availability::Ready;
		    unsigned int side = static_cast<const cxsom::type::Map*>(data.type.get())->side;
		    double coef = 1/(side - 1.0);
		    auto& content = static_cast<cxsom::data::Map&>(data).content;
		    if(data.type->is_Map1D("Scalar") || data.type->is_Map1D("Pos1D")) {
		      unsigned int i = 0;
		      for(auto& e : content) e = coef * (i++);
		    }
		    else if(data.type->is_Map1D("Pos2D")) {
		      unsigned int i = 0;
		      auto it = content.begin();
		      while(it != content.end()) {
			*(it++) = coef * i;
			*(it++) = coef * (i++);
		      }
		    }
		    else if(auto dim = data.type->is_Map1DOfArray(); dim > 0) {
		      double coef = 1/(side - 1.0);
		      unsigned int i = 0;
		      auto it = content.begin();
		      while(it != content.end()) {
			auto dimit_end = it + dim;
			while(it != dimit_end) *(it++) = coef * i;
			++i;
		      }
		    }
		    else if(data.type->is_Map2D("Scalar") || data.type->is_Map2D("Pos1D")) {
		      auto it = content.begin();
		      for(unsigned int i = 0; i < side; ++i)
			for(unsigned int j = 0; j<side; ++j)
			  *(it++) = coef * j;
		    }
		    else if(data.type->is_Map2D("Pos2D")) {
		      auto it = content.begin();
		      for(unsigned int i = 0; i < side; ++i)
			for(unsigned int j = 0; j < side; ++j) {
			  *(it++) = coef * j;
			  *(it++) = coef * j;
			}
		    }
		    else if(auto dim = data.type->is_Map2DOfArray(); dim > 0) {
		      auto it = content.begin();
		      for(unsigned int i = 0; i < side; ++i)
			for(unsigned int j = 0; j<side; ++j) {
			  auto dimit_end = it + dim;
			  while(it != dimit_end)
			    *(it++) = coef * j;
			}
		    }
		    else
		      status = cxsom::data::Availability::Busy;
		  });
      }

      
    }
  
    virtual void send_computation() const override {
      {
	timeline t(name + "_args");
	kwd::type("xi", args[0], CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::at("xi", 0) << fx::clear() | kwd::use("value", .5);
      }
      {
	timeline t(name);
	kwd::type("res", res, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::at("res", 0) << fx::match_gaussian(kwd::at(kwd::var(name + "_args", "xi"), 0),
						kwd::at(kwd::var(name + "_args", "wgt"), 0)) | kwd::use("sigma", sigma);
      }
    }
  
    virtual bool test_result() const override {
      bool error_status = false;

      try {
	cxsom::data::Variable v(root_dir, {name, "res"}, nullptr, std::nullopt, std::nullopt, true);
	auto input_type = cxsom::type::make(args[0]);
	double dim_coef = 1;
	if(input_type->is_Pos2D()) dim_coef = 2;
	else if(auto dim = input_type->is_Array(); dim > 0) dim_coef = dim;
	v[0]->get([&error_status, this, dim_coef](auto status, auto, auto& data) {
		    if(status != cxsom::data::Availability::Ready) {
		      error_status = true;
		      return;
		    }
		    
		    if(data.type->name() != this->res) {
		      error_status = true;
		      return;
		    }

		    if(data.type->is_Map("Scalar")) {
		      auto& content = static_cast<const cxsom::data::Map&>(data).content;
		      auto     side = static_cast<const cxsom::type::Map*>(data.type.get())->side;

		      auto check_pos = [a = 1/(2*sigma*sigma), coef = 1/(side - 1.0), dim_coef](unsigned int i) {return cxsom::match::gaussian(a * dim_coef, i * coef, .5);};
		      
		      if(data.type->is_Map1D()) {
			unsigned int i = 0;
			for(auto e : content)
			  if(fabs(check_pos(i++) - e) > tol) {
			    error_status = true;
			    return;
			  }
			return;
		      }
		      else if(data.type->is_Map2D()) {
			auto it = content.begin();
			for(unsigned int i = 0; i < side; ++i)
			  for(unsigned int j = 0; j < side; ++j)
			    if(fabs(check_pos(j) - *(it++)) > tol) {
			      error_status = true;
			      return;
			    }
			return;
		      }
		      else {
			std::cout << data.type->name() << std::endl;
			error_status = true;
		      }
		      return;
		    }
		    
		    error_status = true;
		    
		  });
      } catch(std::exception& e) {
	std::cout << "Exception : " << e.what() << std::endl;
	error_status = true;
      }
      
      return error_status;
    }
  };

  
  // #################
  // #               # 
  // # LearnTriangle # 
  // #               # 
  // #################
  
  class LearnTriangle : public Base {
  protected:

    double alpha;
    double r;
    double tol;
    
  public:

    LearnTriangle(const fs::path& root_dir,
		  const std::string& name,
		  unsigned int mapdim,
		  const std::string& content,
		  double alpha, double r, double tol)
      : Base(root_dir, name, "learn_triangle", map_of(mapdim, 100, content), {content, map_of(mapdim, 100, content), pos_of(mapdim)}), alpha(alpha), r(r), tol(tol) {}

    virtual void make_args() const override {}
  
    virtual void send_computation() const override {
      {
	timeline t(name + "_args");
	kwd::type("xi", args[0], CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::at("xi", 0) << fx::clear() | kwd::use("value", 1);
	kwd::type("wgt", args[1], CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::at("wgt", 0) << fx::clear() | kwd::use("value", 0);
	kwd::type("bmu", args[2], CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::at("bmu", 0) << fx::clear() | kwd::use("value", .5);
      }
      
      {
	timeline t(name);
	kwd::type("res", res, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::at("res", 0) << fx::learn_triangle(kwd::at(kwd::var(name + "_args", "xi"), 0),
						kwd::at(kwd::var(name + "_args", "wgt"), 0),
						kwd::at(kwd::var(name + "_args", "bmu"), 0)) | kwd::use("r", r), kwd::use("alpha", alpha), kwd::use("tol", 0);
      }
	
    }
  
    virtual bool test_result() const override {
      bool error_status = false;

      try {
	cxsom::data::Variable v(root_dir, {name, "res"}, nullptr, std::nullopt, std::nullopt, true);
	
	auto input_type = cxsom::type::make(args[0]);
	unsigned int dim = 1;
	if(input_type->is_Pos2D()) dim = 2;
	else if(auto d = input_type->is_Array(); d > 0) dim = d;

	v[0]->get([&error_status, this, dim](auto status, auto, auto& data) {
		    if(status != cxsom::data::Availability::Ready) {
		      error_status = true;
		      return;
		    }
		    
		    if(data.type->name() != this->res) {
		      error_status = true;
		      return;
		    }

		    if(data.type->is_Map()) {
		      auto& content = static_cast<const cxsom::data::Map&>(data).content;
		      auto     side = static_cast<const cxsom::type::Map*>(data.type.get())->side;

		      
		      if(data.type->is_Map1D()) {
			auto h_pos = [a = 1/r, coef = 1/(side - 1.0)](unsigned int i) {return cxsom::match::triangle(a, i * coef, .5);};
			auto it = content.begin();
			for(unsigned int i = 0; i < side; ++i) {
			  double a_h = alpha * h_pos(i);
			  for(unsigned int d = 0; d < dim; ++d) 
			    if(fabs(a_h - *(it++)) > tol) {
			      error_status = true;
			      return;
			    }
			}
			return;
		      }
		      else if(data.type->is_Map2D()) {
			auto h_pos = [a = 1/r, coef = 1/(side - 1.0)](unsigned int i, unsigned j) {
				       std::array<double, 2> bmu = {.5, .5};
				       std::array<double, 2> p   = {j * coef, i * coef};
				       return cxsom::match::triangle(2, a, std::data(p), std::data(bmu));
				     };
			auto it = content.begin();
			for(unsigned int i = 0; i < side; ++i)
			  for(unsigned int j = 0; j < side; ++j) {
			    double a_h = alpha * h_pos(i, j);
			    for(unsigned int d = 0; d < dim; ++d) 
			      if(fabs(a_h - *(it++)) > tol) {
				error_status = true;
				return;
			      }
			  }
			return;
		      }
		      else {
			std::cout << data.type->name() << std::endl;
			error_status = true;
		      }
		      return;
		    }
		    
		    error_status = true;
		    
		  });
      } catch(std::exception& e) {
	std::cout << "Exception : " << e.what() << std::endl;
	error_status = true;
      }
      
      return error_status;
    }
  };

  

  
  // #################
  // #               # 
  // # LearnGaussian # 
  // #               # 
  // #################
  
  class LearnGaussian : public Base {
  protected:

    double alpha;
    double sigma;
    double tol;
    
  public:

    LearnGaussian(const fs::path& root_dir,
		  const std::string& name,
		  unsigned int mapdim,
		  const std::string& content,
		  double alpha, double sigma, double tol)
      : Base(root_dir, name, "learn_gaussian", map_of(mapdim, 100, content), {content, map_of(mapdim, 100, content), pos_of(mapdim)}), alpha(alpha), sigma(sigma), tol(tol) {}

    virtual void make_args() const override {}
  
    virtual void send_computation() const override {
      {
	timeline t(name + "_args");
	kwd::type("xi", args[0], CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::at("xi", 0) << fx::clear() | kwd::use("value", 1);
	kwd::type("wgt", args[1], CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::at("wgt", 0) << fx::clear() | kwd::use("value", 0);
	kwd::type("bmu", args[2], CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::at("bmu", 0) << fx::clear() | kwd::use("value", .5);
      }
      
      {
	timeline t(name);
	kwd::type("res", res, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::at("res", 0) << fx::learn_gaussian(kwd::at(kwd::var(name + "_args", "xi"), 0),
						kwd::at(kwd::var(name + "_args", "wgt"), 0),
						kwd::at(kwd::var(name + "_args", "bmu"), 0)) | kwd::use("sigma", sigma), kwd::use("alpha", alpha), kwd::use("tol", 0);
      }
	
    }
  
    virtual bool test_result() const override {
      bool error_status = false;

      try {
	cxsom::data::Variable v(root_dir, {name, "res"}, nullptr, std::nullopt, std::nullopt, true);
	
	auto input_type = cxsom::type::make(args[0]);
	unsigned int dim = 1;
	if(input_type->is_Pos2D()) dim = 2;
	else if(auto d = input_type->is_Array(); d > 0) dim = d;

	v[0]->get([&error_status, this, dim](auto status, auto, auto& data) {
		    if(status != cxsom::data::Availability::Ready) {
		      error_status = true;
		      return;
		    }
		    
		    if(data.type->name() != this->res) {
		      error_status = true;
		      return;
		    }

		    if(data.type->is_Map()) {
		      auto& content = static_cast<const cxsom::data::Map&>(data).content;
		      auto     side = static_cast<const cxsom::type::Map*>(data.type.get())->side;

		      
		      if(data.type->is_Map1D()) {
			auto h_pos = [a = 1/(2*sigma*sigma), coef = 1/(side - 1.0)](unsigned int i) {return cxsom::match::gaussian(a, i * coef, .5);};
			auto it = content.begin();
			for(unsigned int i = 0; i < side; ++i) {
			  double a_h = alpha * h_pos(i);
			  for(unsigned int d = 0; d < dim; ++d) 
			    if(fabs(a_h - *(it++)) > tol) {
			      error_status = true;
			      return;
			    }
			}
			return;
		      }
		      else if(data.type->is_Map2D()) {
			auto h_pos = [a = 1/(2*sigma*sigma), coef = 1/(side - 1.0)](unsigned int i, unsigned j) {
				       std::array<double, 2> bmu = {.5, .5};
				       std::array<double, 2> p   = {j * coef, i * coef};
				       return cxsom::match::gaussian(2, a, std::data(p), std::data(bmu));
				     };
			auto it = content.begin();
			for(unsigned int i = 0; i < side; ++i)
			  for(unsigned int j = 0; j < side; ++j) {
			    double a_h = alpha * h_pos(i, j);
			    for(unsigned int d = 0; d < dim; ++d) 
			      if(fabs(a_h - *(it++)) > tol) {
				error_status = true;
				return;
			      }
			  }
			return;
		      }
		      else {
			std::cout << data.type->name() << std::endl;
			error_status = true;
		      }
		      return;
		    }
		    
		    error_status = true;
		    
		  });
      } catch(std::exception& e) {
	std::cout << "Exception : " << e.what() << std::endl;
	error_status = true;
      }
      
      return error_status;
    }
  };
  

  // ##########
  // #        #
  // # Argmax # 
  // #        #
  // ##########
  
  class Argmax : public Base {
  protected:
  
  public:

    Argmax(const fs::path& root_dir,
	   const std::string& name,
	   unsigned int dim) : Base(root_dir, name, "argmax", pos_of(dim), {map_of(dim, 101, "Scalar")}) {}

    virtual void make_args() const override {
      
      fs::create_directories(root_dir / (name + "_args"));
      
      {
      	cxsom::data::File file(root_dir, {name + "_args", "x"});
      	file.realize(cxsom::type::make(args[0]), CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
      }
      
      {
      	cxsom::data::Variable v(root_dir, {name + "_args", "x"}, nullptr, std::nullopt, std::nullopt, true);
      	v[0]->set([](auto& status, auto&, auto& data) {
      		    status = cxsom::data::Availability::Ready;
      		    unsigned int side = static_cast<const cxsom::type::Map*>(data.type.get())->side;
      		    double coef = 1/(side - 1.0);
      		    auto& content = static_cast<cxsom::data::Map&>(data).content;
      		    if(data.type->is_Map1D("Scalar")) {
      		      unsigned int i = 0;
      		      for(auto& e : content) e = .5 - std::fabs(.5 - coef * (i++));
      		    }
      		    if(data.type->is_Map2D("Scalar")) {
      		      auto it = content.begin();
      		      for(unsigned int i = 0; i < side; ++i) {
      			double y = coef * i;
      			double dy = y - .5;
      			dy *= dy;
      			for(unsigned int j = 0; j < side; ++j) {
      			  double x = coef * j;
      			  double dx = x - .5;
      			  double d = std::min(std::sqrt(dx*dx + dy), .5);
      			  *(it++) = .5 - d;
      			}
      		      }
      		    }
      		  });
      }
    }
  
    virtual void send_computation() const override {
      timeline t(name);
      kwd::type("res", res, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
      kwd::at("res", 0) << fx::argmax(kwd::at(kwd::var(name + "_args", "x"), 0)) | kwd::use("random-bmu", 0);
    }
  
    virtual bool test_result() const override {
      bool error_status = false;

      try {
	cxsom::data::Variable v(root_dir, {name, "res"}, nullptr, std::nullopt, std::nullopt, true);

	v[0]->get([&error_status, this](auto status, auto, auto& data) {
		    if(status != cxsom::data::Availability::Ready) {
		      error_status = true;
		      return;
		    }
		    
		    if(data.type->name() != this->res) {
		      error_status = true;
		      return;
		    }

		    if(data.type->is_Pos1D()) {
		      error_status = (static_cast<const cxsom::data::d1::Pos&>(data).x != .5);
		      return;
		    }
		    
		    if(data.type->is_Pos2D()) {
		      const std::array<double, 2>& xy = static_cast<const cxsom::data::d2::Pos&>(data).xy;
		      error_status = (xy[0] != .5 || xy[1] != .5);
		      return;
		    }
		    
		    error_status = true;
		  });
      } catch(std::exception& e) {
	std::cout << "Exception : " << e.what() << std::endl;
	error_status = true;
      }
      
      return error_status;
    }
  };

  
  

  // ##############
  // #            #
  // # ConvArgmax # 
  // #            #
  // ##############
  
  class ConvArgmax : public Base {
  protected:

    double noise;
    double sigma;
    double tol;
    
  public:

    ConvArgmax(const fs::path& root_dir,
	       const std::string& name,
	       unsigned int dim,
	       double noise,
	       double sigma,
	       double tol)
      : Base(root_dir, name, "conv_argmax", pos_of(dim), {map_of(dim, 101, "Scalar")}),
	noise(noise), sigma(sigma), tol(tol) {}

    virtual void make_args() const override {
      
      fs::create_directories(root_dir / (name + "_args"));
      
      {
      	cxsom::data::File file(root_dir, {name + "_args", "x"});
      	file.realize(cxsom::type::make(args[0]), CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
      }
      
      
      cxsom::data::Variable v(root_dir, {name + "_args", "x"}, nullptr, std::nullopt, std::nullopt, true);
      v[0]->set([this](auto& status, auto&, auto& data) {
		  status = cxsom::data::Availability::Ready;
		  unsigned int side = static_cast<const cxsom::type::Map*>(data.type.get())->side;
		  double coef = 1/(side - 1.0);
		  auto& content = static_cast<cxsom::data::Map&>(data).content;

		  std::random_device rd;
		  std::mt19937 gen(rd());
		  auto U = std::uniform_real_distribution<double>(0., this->noise);
		    
		    
		  if(data.type->is_Map1D("Scalar")) {
		    unsigned int i = 0;
		    for(auto& e : content) e = U(gen) + .5 - std::fabs(.5 - coef * (i++));
		  }
		  if(data.type->is_Map2D("Scalar")) {
		    auto it = content.begin();
		    for(unsigned int i = 0; i < side; ++i) {
		      double y = coef * i;
		      double dy = y - .5;
		      dy *= dy;
		      for(unsigned int j = 0; j < side; ++j) {
			double x = coef * j;
			double dx = x - .5;
			double d = std::min(std::sqrt(dx*dx + dy), .5);
			*(it++) = U(gen) + .5 - d;
		      }
		    }
		  }
		});
      
    }
    
    virtual void send_computation() const override {
      timeline t(name);
      kwd::type("res", res, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
      kwd::at("res", 0) << fx::conv_argmax(kwd::at(kwd::var(name + "_args", "x"), 0)) | kwd::use("random-bmu", 0), kwd::use("sigma", sigma);
    }
  
    virtual bool test_result() const override {
      bool error_status = false;

      try {
	cxsom::data::Variable v(root_dir, {name, "res"}, nullptr, std::nullopt, std::nullopt, true);

	v[0]->get([&error_status, this](auto status, auto, auto& data) {
		    if(status != cxsom::data::Availability::Ready) {
		      error_status = true;
		      return;
		    }
		    
		    if(data.type->name() != this->res) {
		      error_status = true;
		      return;
		    }

		    if(data.type->is_Pos1D()) {
		      error_status = (std::fabs(static_cast<const cxsom::data::d1::Pos&>(data).x - .5) > this->tol);
		      return;
		    }
		    
		    if(data.type->is_Pos2D()) {
		      const std::array<double, 2>& xy = static_cast<const cxsom::data::d2::Pos&>(data).xy;
		      error_status = (std::fabs(xy[0] - .5) > this->tol || std::fabs(xy[1] - .5) > this->tol);
		      return;
		    }
		    
		    error_status = true;
		  });
      } catch(std::exception& e) {
	std::cout << "Exception : " << e.what() << std::endl;
	error_status = true;
      }
      
      return error_status;
    }
  };
  
  // ################
  // #              #
  // # TowardArgmax # 
  // #              #
  // ################
  
  class TowardArgmax : public Base {
  protected:

    double noise;
    double sigma;
    double delta;
    double tol;
    
  public:

    TowardArgmax(const fs::path& root_dir,
		 const std::string& name,
		 unsigned int dim,
		 double noise,
		 double sigma,
		 double delta,
		 double tol)
      : Base(root_dir, name, "toward_argmax", pos_of(dim), {map_of(dim, 101, "Scalar"), pos_of(dim)}),
	noise(noise), sigma(sigma), delta(delta), tol(tol) {}

    virtual void make_args() const override {
      
      fs::create_directories(root_dir / (name + "_args"));

      auto bmu_type = cxsom::type::make(res);

      if(bmu_type->is_Pos1D()) {
	{
	  cxsom::data::File file(root_dir, {name + "_args", "east"});
	  file.realize(bmu_type, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	}
	{
	  cxsom::data::File file(root_dir, {name + "_args", "west"});
	  file.realize(bmu_type, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	}
      	cxsom::data::Variable e(root_dir, {name + "_args", "east"}, nullptr, std::nullopt, std::nullopt, true);
      	cxsom::data::Variable w(root_dir, {name + "_args", "west"}, nullptr, std::nullopt, std::nullopt, true);
	e[0]->set([](auto& status, auto&, auto& data) {
		    status = cxsom::data::Availability::Ready;
		    static_cast<cxsom::data::d1::Pos&>(data).x = 1.;
		  });
	w[0]->set([](auto& status, auto&, auto& data) {
		    status = cxsom::data::Availability::Ready;
		    static_cast<cxsom::data::d1::Pos&>(data).x = 0.;
		  });
      }
      else if(bmu_type->is_Pos2D()) {
	{
	  cxsom::data::File file(root_dir, {name + "_args", "north"});
	  file.realize(bmu_type, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	}
	{
	  cxsom::data::File file(root_dir, {name + "_args", "south"});
	  file.realize(bmu_type, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	}
	{
	  cxsom::data::File file(root_dir, {name + "_args", "east"});
	  file.realize(bmu_type, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	}
	{
	  cxsom::data::File file(root_dir, {name + "_args", "west"});
	  file.realize(bmu_type, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	}
	
      	cxsom::data::Variable n(root_dir, {name + "_args", "north"}, nullptr, std::nullopt, std::nullopt, true);
      	cxsom::data::Variable s(root_dir, {name + "_args", "south"}, nullptr, std::nullopt, std::nullopt, true);
      	cxsom::data::Variable e(root_dir, {name + "_args", "east"},  nullptr, std::nullopt, std::nullopt, true);
      	cxsom::data::Variable w(root_dir, {name + "_args", "west"},  nullptr, std::nullopt, std::nullopt, true);
	n[0]->set([](auto& status, auto&, auto& data) {
		    status = cxsom::data::Availability::Ready;
		    static_cast<cxsom::data::d2::Pos&>(data).xy = {.5, 0.};
		  });
	s[0]->set([](auto& status, auto&, auto& data) {
		    status = cxsom::data::Availability::Ready;
		    static_cast<cxsom::data::d2::Pos&>(data).xy = {.5, 1.};
		  });
	e[0]->set([](auto& status, auto&, auto& data) {
		    status = cxsom::data::Availability::Ready;
		    static_cast<cxsom::data::d2::Pos&>(data).xy = {1., .5};
		  });
	w[0]->set([](auto& status, auto&, auto& data) {
		    status = cxsom::data::Availability::Ready;
		    static_cast<cxsom::data::d2::Pos&>(data).xy = {0., .5};
		  });
      }
      

      {
	cxsom::data::File file(root_dir, {name + "_args", "x"});
	file.realize(cxsom::type::make(args[0]), CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
      }
      cxsom::data::Variable v(root_dir, {name + "_args", "x"}, nullptr, std::nullopt, std::nullopt, true);
      v[0]->set([this](auto& status, auto&, auto& data) {
		  status = cxsom::data::Availability::Ready;
		  unsigned int side = static_cast<const cxsom::type::Map*>(data.type.get())->side;
		  double coef = 1/(side - 1.0);
		  auto& content = static_cast<cxsom::data::Map&>(data).content;

		  std::random_device rd;
		  std::mt19937 gen(rd());
		  auto U = std::uniform_real_distribution<double>(0., this->noise);
		  
		  
		  if(data.type->is_Map1D("Scalar")) {
		    unsigned int i = 0;
		    for(auto& e : content) e = U(gen) + .5 - std::fabs(.5 - coef * (i++));
		  }
		  if(data.type->is_Map2D("Scalar")) {
		    auto it = content.begin();
		    for(unsigned int i = 0; i < side; ++i) {
		      double y = coef * i;
		      double dy = y - .5;
		      dy *= dy;
		      for(unsigned int j = 0; j < side; ++j) {
			double x = coef * j;
			double dx = x - .5;
			double d = std::min(std::sqrt(dx*dx + dy), .5);
			*(it++) = U(gen) + .5 - d;
		      }
		    }
		  }
		});
    }
  
  
    virtual void send_computation() const override {
      auto bmu_type = cxsom::type::make(res);
      timeline t(name);
      
      if(bmu_type->is_Pos1D() || bmu_type->is_Pos2D()) {
	kwd::type("east",  res, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::type("west",  res, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::at("east",  0) << fx::toward_argmax(kwd::at(kwd::var(name + "_args", "x"), 0), kwd::at(kwd::var(name + "_args", "east"),  0))   | kwd::use("random-bmu", 0), kwd::use("sigma", sigma), kwd::use("delta", delta);
	kwd::at("west",  0) << fx::toward_argmax(kwd::at(kwd::var(name + "_args", "x"), 0), kwd::at(kwd::var(name + "_args", "west"),  0))   | kwd::use("random-bmu", 0), kwd::use("sigma", sigma), kwd::use("delta", delta);
      }
      
      if(bmu_type->is_Pos2D()) {
	kwd::type("north", res, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::type("south", res, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::at("north", 0) << fx::toward_argmax(kwd::at(kwd::var(name + "_args", "x"), 0), kwd::at(kwd::var(name + "_args", "north"), 0))   | kwd::use("random-bmu", 0), kwd::use("sigma", sigma), kwd::use("delta", delta);
	kwd::at("south", 0) << fx::toward_argmax(kwd::at(kwd::var(name + "_args", "x"), 0), kwd::at(kwd::var(name + "_args", "south"), 0))   | kwd::use("random-bmu", 0), kwd::use("sigma", sigma), kwd::use("delta", delta);
      }
      
    }
  
    virtual bool test_result() const override {
      bool error_status = false;

      try {
	auto bmu_type = cxsom::type::make(res);

	if(bmu_type->is_Pos1D()) {
	  cxsom::data::Variable e(root_dir, {name, "east"}, nullptr, std::nullopt, std::nullopt, true);
	  e[0]->get([&error_status, this](auto status, auto, auto& data) {
		      if(status != cxsom::data::Availability::Ready) {
			error_status = true;
			return;
		      }
		      
		      if(data.type->name() != this->args[1]) {
			error_status = true;
			return;
		      }

		      double x = static_cast<const cxsom::data::d1::Pos&>(data).x;
		      error_status = std::fabs((1.-this->delta) - x) > this->tol;
		    });
	  
	  cxsom::data::Variable w(root_dir, {name, "west"}, nullptr, std::nullopt, std::nullopt, true);
	  w[0]->get([&error_status, this](auto status, auto, auto& data) {
		      if(status != cxsom::data::Availability::Ready) {
			error_status = true;
			return;
		      }
		      
		      if(data.type->name() != this->args[1]) {
			error_status = true;
			return;
		      }

		      double x = static_cast<const cxsom::data::d1::Pos&>(data).x;
		      error_status = std::fabs(this->delta - x) > this->tol;
		    });
	}
	else if(bmu_type->is_Pos2D()) {
	  cxsom::data::Variable e(root_dir, {name, "east"}, nullptr, std::nullopt, std::nullopt, true);
	  e[0]->get([&error_status, this](auto status, auto, auto& data) {
		      if(status != cxsom::data::Availability::Ready) {
			error_status = true;
			return;
		      }
		      
		      if(data.type->name() != this->args[1]) {
			error_status = true;
			return;
		      }

		      auto& xy = static_cast<const cxsom::data::d2::Pos&>(data).xy;
		      error_status = (std::fabs((1.-this->delta) - xy[0]) > this->tol || std::fabs(.5 - xy[1]) > this->tol);
		    });
	  
	  if(!error_status) {
	    cxsom::data::Variable w(root_dir, {name, "west"}, nullptr, std::nullopt, std::nullopt, true);
	    w[0]->get([&error_status, this](auto status, auto, auto& data) {
			if(status != cxsom::data::Availability::Ready) {
			  error_status = true;
			  return;
			}
		      
			if(data.type->name() != this->args[1]) {
			  error_status = true;
			  return;
			}

			auto& xy = static_cast<const cxsom::data::d2::Pos&>(data).xy;
			error_status = (std::fabs(this->delta - xy[0]) > this->tol || std::fabs(.5 - xy[1]) > this->tol);
		      });
	  }
	  
	  if(!error_status){	  
	    cxsom::data::Variable n(root_dir, {name, "north"}, nullptr, std::nullopt, std::nullopt, true);
	    n[0]->get([&error_status, this](auto status, auto, auto& data) {
			if(status != cxsom::data::Availability::Ready) {
			  error_status = true;
			  return;
			}
		      
			if(data.type->name() != this->args[1]) {
			  error_status = true;
			  return;
			}

			auto& xy = static_cast<const cxsom::data::d2::Pos&>(data).xy;
			error_status = (std::fabs(.5 - xy[0]) > this->tol || std::fabs(this->delta - xy[1]) > this->tol);
		      });
	  }

	  
	  if(!error_status) {
	    cxsom::data::Variable s(root_dir, {name, "south"}, nullptr, std::nullopt, std::nullopt, true);
	    s[0]->get([&error_status, this](auto status, auto, auto& data) {
			if(status != cxsom::data::Availability::Ready) {
			  error_status = true;
			  return;
			}
		      
			if(data.type->name() != this->args[1]) {
			  error_status = true;
			  return;
			}

			auto& xy = static_cast<const cxsom::data::d2::Pos&>(data).xy;
			error_status = (std::fabs(.5 - xy[0]) > this->tol || std::fabs((1. - this->delta) - xy[1]) > this->tol);
		      });
	  }
	}
	else
	  error_status = true;
      } catch(std::exception& e) {
	std::cout << "Exception : " << e.what() << std::endl;
	error_status = true;
      }
      
      return error_status;
    }
  };
  
  // ####################
  // #                  #
  // # TowardConvArgmax # 
  // #                  #
  // ####################
  
  class TowardConvArgmax : public Base {
  protected:

    double noise;
    double sigma;
    double delta;
    double tol;
    
  public:

    TowardConvArgmax(const fs::path& root_dir,
		     const std::string& name,
		     unsigned int dim,
		     double noise,
		     double sigma,
		     double delta,
		     double tol)
      : Base(root_dir, name, "toward_conv_argmax", pos_of(dim), {map_of(dim, 101, "Scalar"), pos_of(dim)}),
	noise(noise), sigma(sigma), delta(delta), tol(tol) {}

    virtual void make_args() const override {
      
      fs::create_directories(root_dir / (name + "_args"));

      auto bmu_type = cxsom::type::make(res);

      if(bmu_type->is_Pos1D()) {
	{
	  cxsom::data::File file(root_dir, {name + "_args", "east"});
	  file.realize(bmu_type, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	}
	{
	  cxsom::data::File file(root_dir, {name + "_args", "west"});
	  file.realize(bmu_type, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	}
      	cxsom::data::Variable e(root_dir, {name + "_args", "east"}, nullptr, std::nullopt, std::nullopt, true);
      	cxsom::data::Variable w(root_dir, {name + "_args", "west"}, nullptr, std::nullopt, std::nullopt, true);
	e[0]->set([](auto& status, auto&, auto& data) {
		    status = cxsom::data::Availability::Ready;
		    static_cast<cxsom::data::d1::Pos&>(data).x = 1.;
		  });
	w[0]->set([](auto& status, auto&, auto& data) {
		    status = cxsom::data::Availability::Ready;
		    static_cast<cxsom::data::d1::Pos&>(data).x = 0.;
		  });
      }
      else if(bmu_type->is_Pos2D()) {
	{
	  cxsom::data::File file(root_dir, {name + "_args", "north"});
	  file.realize(bmu_type, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	}
	{
	  cxsom::data::File file(root_dir, {name + "_args", "south"});
	  file.realize(bmu_type, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	}
	{
	  cxsom::data::File file(root_dir, {name + "_args", "east"});
	  file.realize(bmu_type, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	}
	{
	  cxsom::data::File file(root_dir, {name + "_args", "west"});
	  file.realize(bmu_type, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	}
	
      	cxsom::data::Variable n(root_dir, {name + "_args", "north"}, nullptr, std::nullopt, std::nullopt, true);
      	cxsom::data::Variable s(root_dir, {name + "_args", "south"}, nullptr, std::nullopt, std::nullopt, true);
      	cxsom::data::Variable e(root_dir, {name + "_args", "east"},  nullptr, std::nullopt, std::nullopt, true);
      	cxsom::data::Variable w(root_dir, {name + "_args", "west"},  nullptr, std::nullopt, std::nullopt, true);
	n[0]->set([](auto& status, auto&, auto& data) {
		    status = cxsom::data::Availability::Ready;
		    static_cast<cxsom::data::d2::Pos&>(data).xy = {.5, 0.};
		  });
	s[0]->set([](auto& status, auto&, auto& data) {
		    status = cxsom::data::Availability::Ready;
		    static_cast<cxsom::data::d2::Pos&>(data).xy = {.5, 1.};
		  });
	e[0]->set([](auto& status, auto&, auto& data) {
		    status = cxsom::data::Availability::Ready;
		    static_cast<cxsom::data::d2::Pos&>(data).xy = {1., .5};
		  });
	w[0]->set([](auto& status, auto&, auto& data) {
		    status = cxsom::data::Availability::Ready;
		    static_cast<cxsom::data::d2::Pos&>(data).xy = {0., .5};
		  });
      }
      

      {
	cxsom::data::File file(root_dir, {name + "_args", "x"});
	file.realize(cxsom::type::make(args[0]), CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
      }
      cxsom::data::Variable v(root_dir, {name + "_args", "x"}, nullptr, std::nullopt, std::nullopt, true);
      v[0]->set([this](auto& status, auto&, auto& data) {
		  status = cxsom::data::Availability::Ready;
		  unsigned int side = static_cast<const cxsom::type::Map*>(data.type.get())->side;
		  double coef = 1/(side - 1.0);
		  auto& content = static_cast<cxsom::data::Map&>(data).content;

		  std::random_device rd;
		  std::mt19937 gen(rd());
		  auto U = std::uniform_real_distribution<double>(0., this->noise);
		  
		  
		  if(data.type->is_Map1D("Scalar")) {
		    unsigned int i = 0;
		    for(auto& e : content) e = U(gen) + .5 - std::fabs(.5 - coef * (i++));
		  }
		  if(data.type->is_Map2D("Scalar")) {
		    auto it = content.begin();
		    for(unsigned int i = 0; i < side; ++i) {
		      double y = coef * i;
		      double dy = y - .5;
		      dy *= dy;
		      for(unsigned int j = 0; j < side; ++j) {
			double x = coef * j;
			double dx = x - .5;
			double d = std::min(std::sqrt(dx*dx + dy), .5);
			*(it++) = U(gen) + .5 - d;
		      }
		    }
		  }
		});
    }
  
  
    virtual void send_computation() const override {
      auto bmu_type = cxsom::type::make(res);
      timeline t(name);
      
      if(bmu_type->is_Pos1D() || bmu_type->is_Pos2D()) {
	kwd::type("east",  res, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::type("west",  res, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::at("east",  0) << fx::toward_conv_argmax(kwd::at(kwd::var(name + "_args", "x"), 0), kwd::at(kwd::var(name + "_args", "east"),  0))   | kwd::use("random-bmu", 0), kwd::use("sigma", sigma), kwd::use("delta", delta);
	kwd::at("west",  0) << fx::toward_conv_argmax(kwd::at(kwd::var(name + "_args", "x"), 0), kwd::at(kwd::var(name + "_args", "west"),  0))   | kwd::use("random-bmu", 0), kwd::use("sigma", sigma), kwd::use("delta", delta);
      }
      
      if(bmu_type->is_Pos2D()) {
	kwd::type("north", res, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::type("south", res, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::at("north", 0) << fx::toward_conv_argmax(kwd::at(kwd::var(name + "_args", "x"), 0), kwd::at(kwd::var(name + "_args", "north"), 0))   | kwd::use("random-bmu", 0), kwd::use("sigma", sigma), kwd::use("delta", delta);
	kwd::at("south", 0) << fx::toward_conv_argmax(kwd::at(kwd::var(name + "_args", "x"), 0), kwd::at(kwd::var(name + "_args", "south"), 0))   | kwd::use("random-bmu", 0), kwd::use("sigma", sigma), kwd::use("delta", delta);
      }
      
    }
  
    virtual bool test_result() const override {
      bool error_status = false;

      try {
	auto bmu_type = cxsom::type::make(res);

	if(bmu_type->is_Pos1D()) {
	  cxsom::data::Variable e(root_dir, {name, "east"}, nullptr, std::nullopt, std::nullopt, true);
	  e[0]->get([&error_status, this](auto status, auto, auto& data) {
		      if(status != cxsom::data::Availability::Ready) {
			error_status = true;
			return;
		      }
		      
		      if(data.type->name() != this->args[1]) {
			error_status = true;
			return;
		      }

		      double x = static_cast<const cxsom::data::d1::Pos&>(data).x;
		      error_status = std::fabs((1.-this->delta) - x) > this->tol;
		    });
	  
	  cxsom::data::Variable w(root_dir, {name, "west"}, nullptr, std::nullopt, std::nullopt, true);
	  w[0]->get([&error_status, this](auto status, auto, auto& data) {
		      if(status != cxsom::data::Availability::Ready) {
			error_status = true;
			return;
		      }
		      
		      if(data.type->name() != this->args[1]) {
			error_status = true;
			return;
		      }

		      double x = static_cast<const cxsom::data::d1::Pos&>(data).x;
		      error_status = std::fabs(this->delta - x) > this->tol;
		    });
	}
	else if(bmu_type->is_Pos2D()) {
	  cxsom::data::Variable e(root_dir, {name, "east"}, nullptr, std::nullopt, std::nullopt, true);
	  e[0]->get([&error_status, this](auto status, auto, auto& data) {
		      if(status != cxsom::data::Availability::Ready) {
			error_status = true;
			return;
		      }
		      
		      if(data.type->name() != this->args[1]) {
			error_status = true;
			return;
		      }

		      auto& xy = static_cast<const cxsom::data::d2::Pos&>(data).xy;
		      error_status = (std::fabs((1.-this->delta) - xy[0]) > this->tol || std::fabs(.5 - xy[1]) > this->tol);
		    });
	  
	  if(!error_status) {
	    cxsom::data::Variable w(root_dir, {name, "west"}, nullptr, std::nullopt, std::nullopt, true);
	    w[0]->get([&error_status, this](auto status, auto, auto& data) {
			if(status != cxsom::data::Availability::Ready) {
			  error_status = true;
			  return;
			}
		      
			if(data.type->name() != this->args[1]) {
			  error_status = true;
			  return;
			}

			auto& xy = static_cast<const cxsom::data::d2::Pos&>(data).xy;
			error_status = (std::fabs(this->delta - xy[0]) > this->tol || std::fabs(.5 - xy[1]) > this->tol);
		      });
	  }
	  
	  if(!error_status){	  
	    cxsom::data::Variable n(root_dir, {name, "north"}, nullptr, std::nullopt, std::nullopt, true);
	    n[0]->get([&error_status, this](auto status, auto, auto& data) {
			if(status != cxsom::data::Availability::Ready) {
			  error_status = true;
			  return;
			}
		      
			if(data.type->name() != this->args[1]) {
			  error_status = true;
			  return;
			}

			auto& xy = static_cast<const cxsom::data::d2::Pos&>(data).xy;
			error_status = (std::fabs(.5 - xy[0]) > this->tol || std::fabs(this->delta - xy[1]) > this->tol);
		      });
	  }

	  
	  if(!error_status) {
	    cxsom::data::Variable s(root_dir, {name, "south"}, nullptr, std::nullopt, std::nullopt, true);
	    s[0]->get([&error_status, this](auto status, auto, auto& data) {
			if(status != cxsom::data::Availability::Ready) {
			  error_status = true;
			  return;
			}
		      
			if(data.type->name() != this->args[1]) {
			  error_status = true;
			  return;
			}

			auto& xy = static_cast<const cxsom::data::d2::Pos&>(data).xy;
			error_status = (std::fabs(.5 - xy[0]) > this->tol || std::fabs((1. - this->delta) - xy[1]) > this->tol);
		      });
	  }
	}
	else
	  error_status = true;
      } catch(std::exception& e) {
	std::cout << "Exception : " << e.what() << std::endl;
	error_status = true;
      }
      
      return error_status;
    }
  };
  
  // ###########
  // #         # 
  // # ValueAt # 
  // #         # 
  // ###########
  
  class ValueAt : public Base {
  protected:
    
  public:

    ValueAt(const fs::path& root_dir,
		  const std::string& name,
		  unsigned int mapdim,
		  const std::string& content)
      : Base(root_dir, name, "value_at", content, {map_of(mapdim, 100, content), pos_of(mapdim)}) {}

    virtual void make_args() const override {}
  
    virtual void send_computation() const override {
      {
	timeline t(name + "_args");
	kwd::type("collection", args[0], CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::type("index", args[1], CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	kwd::at("collection", 0) << fx::random();
	"index" << fx::random() | kwd::use("walltime", WALLTIME);
      }
      
      {
	timeline t(name);
	kwd::type("res", res, CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
	"res" << fx::value_at(kwd::at(kwd::var(name + "_args", "collection"), 0), kwd::var(name + "_args", "index")) | kwd::use("walltime", WALLTIME);
      }
    }
  
    virtual bool test_result() const override {
      bool error_status = false;
      
      std::vector<double> collection_Scalar;
      
      std::vector<double> res_Scalar;
      std::vector<double> res_Pos1D;
      std::vector<std::array<double, 2>> res_Pos2D;
      std::vector<std::vector<double>> res_Array;
      
      std::vector<double> indices_Pos1D;
      std::vector<std::array<double, 2>> indices_Pos2D;

      try {
	cxsom::data::Variable res(root_dir, {name, "res"}, nullptr, std::nullopt, std::nullopt, true);
	cxsom::data::Variable collection(root_dir, {name + "_args", "collection"}, nullptr, std::nullopt, std::nullopt, true);
	cxsom::data::Variable index(root_dir, {name + "_args", "index"}, nullptr, std::nullopt, std::nullopt, true);

	auto history_size = res.history_length();
	
	// Getting indices
	
	auto index_type = index.get_type();
	if(index_type->is_Pos1D()) {
	  for(std::size_t at = 0; at < history_size; ++at)
	    index[at]->get([&indices_Pos1D](auto status, auto, auto& data) {
	      if(status == cxsom::data::Availability::Ready)
		indices_Pos1D.push_back(static_cast<const cxsom::data::d1::Pos&>(data).x);
	      else
		throw std::runtime_error("Busy slot found in index");
	    });
	}
	else if (index_type->is_Pos2D()) {
	  for(std::size_t at = 0; at < history_size; ++at)
	    index[at]->get([&indices_Pos2D](auto status, auto, auto& data) {
	      if(status == cxsom::data::Availability::Ready)
		indices_Pos2D.push_back(static_cast<const cxsom::data::d2::Pos&>(data).xy);
	      else
		throw std::runtime_error("Busy slot found in index");
	    });
	}
	else {
	  std::ostringstream ostr;
	  ostr << "Bad type found for index variable (" << index_type->name() << ").";
	  throw std::runtime_error(ostr.str());
	}
	

	// Getting results
	
	auto res_type = res.get_type();
	if(res_type->is_Scalar()) {
	  for(std::size_t at = 0; at < history_size; ++at)
	    res[at]->get([&res_Scalar](auto status, auto, auto& data) {
	      if(status == cxsom::data::Availability::Ready)
		res_Scalar.push_back(static_cast<const cxsom::data::Scalar&>(data).value);
	      else
		throw std::runtime_error("Busy slot found in res");
	    });
	}
	else if(res_type->is_Pos1D()) {
	  for(std::size_t at = 0; at < history_size; ++at)
	    res[at]->get([&res_Pos1D](auto status, auto, auto& data) {
	      if(status == cxsom::data::Availability::Ready)
		res_Pos1D.push_back(static_cast<const cxsom::data::d1::Pos&>(data).x);
	      else
		throw std::runtime_error("Busy slot found in res");
	    });
	}
	else if(res_type->is_Pos2D()) {
	  for(std::size_t at = 0; at < history_size; ++at)
	    res[at]->get([&res_Pos2D](auto status, auto, auto& data) {
	      if(status == cxsom::data::Availability::Ready)
		res_Pos2D.push_back(static_cast<const cxsom::data::d2::Pos&>(data).xy);
	      else
		throw std::runtime_error("Busy slot found in res");
	    });
	}
	else if(res_type->is_Array()) {
	  for(std::size_t at = 0; at < history_size; ++at)
	    res[at]->get([&res_Array](auto status, auto, auto& data) {
	      if(status == cxsom::data::Availability::Ready)
		res_Array.push_back(static_cast<const cxsom::data::Array&>(data).content);
	      else
		throw std::runtime_error("Busy slot found in res");
	    });
	}
	else
	  error_status = true;
	
     
      } catch(std::exception& e) {
	std::cout << "Exception : " << e.what() << std::endl;
	error_status = true;
      }
      
      return error_status;
    }
  };

  
  // #######
  // #     # 
  // # All # 
  // #     # 
  // #######

#define TOLERANCE 1e-8
  
  struct All {
    std::vector<Base*> tests;
    All(const fs::path& root_dir) : tests() {
      auto out = std::back_inserter(tests);

      /*
      *(out++) = new Clear(root_dir, "clear_Scalar",       "Scalar",             .5);
      *(out++) = new Clear(root_dir, "clear_Pos1D",        "Pos1D",              .5);
      *(out++) = new Clear(root_dir, "clear_Pos2D",        "Pos2D",              .5);
      *(out++) = new Clear(root_dir, "clear_Array",        "Array=10",           .5);
      *(out++) = new Clear(root_dir, "clear_Map1D_Scalar", "Map1D<Scalar>=10",   .5);
      *(out++) = new Clear(root_dir, "clear_Map1D_Pos1D",  "Map1D<Pos1D>=10",    .5);
      *(out++) = new Clear(root_dir, "clear_Map1D_Pos2D",  "Map1D<Pos2D>=10",    .5);
      *(out++) = new Clear(root_dir, "clear_Map1D_Array",  "Map1D<Array=10>=10", .5);
      *(out++) = new Clear(root_dir, "clear_Map2D_Scalar", "Map2D<Scalar>=10",   .5);
      *(out++) = new Clear(root_dir, "clear_Map2D_Pos1D",  "Map2D<Pos1D>=10",    .5);
      *(out++) = new Clear(root_dir, "clear_Map2D_Pos2D",  "Map2D<Pos2D>=10",    .5);
      *(out++) = new Clear(root_dir, "clear_Map2D_Array",  "Map2D<Array=10>=10", .5);
      
      *(out++) = new Copy(root_dir, "copy_Scalar",       "Scalar"            );
      *(out++) = new Copy(root_dir, "copy_Pos1D",        "Pos1D"             );
      *(out++) = new Copy(root_dir, "copy_Pos2D",        "Pos2D"             );
      *(out++) = new Copy(root_dir, "copy_Array",        "Array=10"          );
      *(out++) = new Copy(root_dir, "copy_Map1D_Scalar", "Map1D<Scalar>=10"  );
      *(out++) = new Copy(root_dir, "copy_Map1D_Pos1D",  "Map1D<Pos1D>=10"   );
      *(out++) = new Copy(root_dir, "copy_Map1D_Pos2D",  "Map1D<Pos2D>=10"   );
      *(out++) = new Copy(root_dir, "copy_Map1D_Array",  "Map1D<Array=10>=10");
      *(out++) = new Copy(root_dir, "copy_Map2D_Scalar", "Map2D<Scalar>=10"  );
      *(out++) = new Copy(root_dir, "copy_Map2D_Pos1D",  "Map2D<Pos1D>=10"   );
      *(out++) = new Copy(root_dir, "copy_Map2D_Pos2D",  "Map2D<Pos2D>=10"   );
      *(out++) = new Copy(root_dir, "copy_Map2D_Array",  "Map2D<Array=10>=10");
      
      *(out++) = new Average(root_dir, "average_Map1D_Scalar", "Map1D<Scalar>=10");
      *(out++) = new Average(root_dir, "average_Map2D_Scalar", "Map2D<Scalar>=10");
      
      *(out++) = new Merge(root_dir, "merge_Map1D_Scalar", "Map1D<Scalar>=10", .3);
      *(out++) = new Merge(root_dir, "merge_Map2D_Scalar", "Map2D<Scalar>=10", .3);
      
      *(out++) = new MatchTriangle(root_dir, "matchT_1D_Scalar", 1, "Scalar",   .1, TOLERANCE);
      *(out++) = new MatchTriangle(root_dir, "matchT_1D_Pos1D",  1, "Pos1D",    .1, TOLERANCE);
      *(out++) = new MatchTriangle(root_dir, "matchT_1D_Pos2D",  1, "Pos2D",    .1, TOLERANCE);
      *(out++) = new MatchTriangle(root_dir, "matchT_1D_Array",  1, "Array=10", .1, TOLERANCE);
      *(out++) = new MatchTriangle(root_dir, "matchT_2D_Scalar", 2, "Scalar",   .1, TOLERANCE);
      *(out++) = new MatchTriangle(root_dir, "matchT_2D_Pos1D",  2, "Pos1D",    .1, TOLERANCE);
      *(out++) = new MatchTriangle(root_dir, "matchT_2D_Pos2D",  2, "Pos2D",    .1, TOLERANCE);
      *(out++) = new MatchTriangle(root_dir, "matchT_2D_Array",  2, "Array=10", .1, TOLERANCE);
      
      *(out++) = new MatchGaussian(root_dir, "matchG_1D_Scalar", 1, "Scalar",   .1, TOLERANCE);
      *(out++) = new MatchGaussian(root_dir, "matchG_1D_Pos1D",  1, "Pos1D",    .1, TOLERANCE);
      *(out++) = new MatchGaussian(root_dir, "matchG_1D_Pos2D",  1, "Pos2D",    .1, TOLERANCE);
      *(out++) = new MatchGaussian(root_dir, "matchG_1D_Array",  1, "Array=10", .1, TOLERANCE);
      *(out++) = new MatchGaussian(root_dir, "matchG_2D_Scalar", 2, "Scalar",   .1, TOLERANCE);
      *(out++) = new MatchGaussian(root_dir, "matchG_2D_Pos1D",  2, "Pos1D",    .1, TOLERANCE);
      *(out++) = new MatchGaussian(root_dir, "matchG_2D_Pos2D",  2, "Pos2D",    .1, TOLERANCE);
      *(out++) = new MatchGaussian(root_dir, "matchG_2D_Array",  2, "Array=10", .1, TOLERANCE);
      
      *(out++) = new LearnTriangle(root_dir, "learnT_1D_Scalar", 1, "Scalar",   .5, .2, TOLERANCE);
      *(out++) = new LearnTriangle(root_dir, "learnT_1D_Pos1D",  1, "Pos1D",    .5, .2, TOLERANCE);
      *(out++) = new LearnTriangle(root_dir, "learnT_1D_Pos2D",  1, "Pos2D",    .5, .2, TOLERANCE);
      *(out++) = new LearnTriangle(root_dir, "learnT_1D_Array",  1, "Array=10", .5, .2, TOLERANCE);
      *(out++) = new LearnTriangle(root_dir, "learnT_2D_Scalar", 2, "Scalar",   .5, .2, TOLERANCE);
      *(out++) = new LearnTriangle(root_dir, "learnT_2D_Pos1D",  2, "Pos1D",    .5, .2, TOLERANCE);
      *(out++) = new LearnTriangle(root_dir, "learnT_2D_Pos2D",  2, "Pos2D",    .5, .2, TOLERANCE);
      *(out++) = new LearnTriangle(root_dir, "learnT_2D_Array",  2, "Array=10", .5, .2, TOLERANCE);
      
      *(out++) = new LearnGaussian(root_dir, "learnG_1D_Scalar", 1, "Scalar",   .5, .2, TOLERANCE);
      *(out++) = new LearnGaussian(root_dir, "learnG_1D_Pos1D",  1, "Pos1D",    .5, .2, TOLERANCE);
      *(out++) = new LearnGaussian(root_dir, "learnG_1D_Pos2D",  1, "Pos2D",    .5, .2, TOLERANCE);
      *(out++) = new LearnGaussian(root_dir, "learnG_1D_Array",  1, "Array=10", .5, .2, TOLERANCE);
      *(out++) = new LearnGaussian(root_dir, "learnG_2D_Scalar", 2, "Scalar",   .5, .2, TOLERANCE);
      *(out++) = new LearnGaussian(root_dir, "learnG_2D_Pos1D",  2, "Pos1D",    .5, .2, TOLERANCE);
      *(out++) = new LearnGaussian(root_dir, "learnG_2D_Pos2D",  2, "Pos2D",    .5, .2, TOLERANCE);
      *(out++) = new LearnGaussian(root_dir, "learnG_2D_Array",  2, "Array=10", .5, .2, TOLERANCE);
      
      *(out++) = new Argmax(root_dir, "argmax_1D",  1);
      *(out++) = new Argmax(root_dir, "argmax_2D",  2);
      
      *(out++) = new ConvArgmax(root_dir, "conv_argmax_1D",  1, .1, .1, .05);
      *(out++) = new ConvArgmax(root_dir, "conv_argmax_2D",  2, .1, .1, .05);

      
      *(out++) = new TowardArgmax(root_dir, "toward_argmax_1D",  1, .1, .1, .3, .05);
      *(out++) = new TowardArgmax(root_dir, "toward_argmax_2D",  2, .1, .1, .3, .05);
      
      
      *(out++) = new TowardConvArgmax(root_dir, "toward_conv_argmax_1D",  1, .1, .1, .3, .05);
      *(out++) = new TowardConvArgmax(root_dir, "toward_conv_argmax_2D",  2, .1, .1, .3, .05);
      */
      *(out++) = new ValueAt(root_dir, "valueat_1D_Scalar", 1, "Scalar"  );
      *(out++) = new ValueAt(root_dir, "valueat_1D_Pos1D",  1, "Pos1D"   );
      *(out++) = new ValueAt(root_dir, "valueat_1D_Pos2D",  1, "Pos2D"   );
      *(out++) = new ValueAt(root_dir, "valueat_1D_Array",  1, "Array=10");
      *(out++) = new ValueAt(root_dir, "valueat_2D_Scalar", 2, "Scalar"  );
      *(out++) = new ValueAt(root_dir, "valueat_2D_Pos1D",  2, "Pos1D"   );
      *(out++) = new ValueAt(root_dir, "valueat_2D_Pos2D",  2, "Pos2D"   );
      *(out++) = new ValueAt(root_dir, "valueat_2D_Array",  2, "Array=10");
    }
   

    void make_args() const {
      for(auto test_ptr : tests) test_ptr->make_args();
    }
    
    void send_computation() const {
      for(auto test_ptr : tests) test_ptr->send_computation();
    }

    void test_result() const {
      std::vector<std::string> failed;
      auto out = std::back_inserter(failed);
      for(auto test_ptr : tests) if(test_ptr->compute_error_status()) *(out++) = std::string(*test_ptr);
      std::cout << std::endl;
      if(failed.size() == 0)
	std::cout << "All tests passed. Congratulations." << std::endl << std::endl;
      else {
	std::cout << failed.size() << " test(s) failed." << std::endl;
	for(auto& n : failed) std::cout << "  " << n << std::endl;
	std::cout << std::endl;
      }
    }
  };

  
}

// ########
// #      # 
// # Main # 
// #      # 
// ########

int main(int argc, char* argv[]) {

  
  if(argc <= 1) {
    std::cout << "Usage :" << std::endl
	      << "  1 - create a <root-dir> directory, or empty an existing one." << std::endl
	      << "  2 - run : " << argv[0] << " setup <root-dir>" << std::endl
	      << "  3 - run : cxsom-processor <root-dir> <nb-threads> <port>" << std::endl
	      << "  4 - run : " << argv[0] << " send <host> <port>" << std::endl
	      << "  5 - run : " << argv[0] << " check <root-dir>" << std::endl;
    return 0;
  }
  
  std::string command = argv[1];

  if(command == "setup") {
    test::All all(argv[2]);
    all.make_args();
  }
  else if(command == "send") {
    test::All all(".");

    std::vector<std::string> args = {std::string("client"),
				     "send", argv[2], argv[3]};
    context c(args.begin(), args.end());

    all.send_computation();
  }
  else if(command == "check") {
    test::All all(argv[2]);
    all.test_result();
  }
  else 
    std::cout << std::endl
	      << "Bad command " << argv[1] << '.' << std::endl;

              
  return 0;
}
