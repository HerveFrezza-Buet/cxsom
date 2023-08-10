#include <string>
#include <algorithm>
#include <iostream>

#define cxsomLOG
#include <cxsom-server.hpp>
// This is used in case of some macros are set. You can remove this
// lines otherwise... or keep it.
cxsom::Tick*    cxsom::ticker  = new cxsom::Tick();
cxsom::Log*     cxsom::logger  = new cxsom::Log();
cxsom::Monitor* cxsom::monitor = new cxsom::Monitor();

#include <filesystem>
namespace fs = std::filesystem;

// This update performs an addition of all its arguments.
class Addition : public cxsom::update::Base {
private:
  std::optional<double> sum_out; // This is set when all "out" arguments are ok.
  double sum_in     = 0;         // This is the summation of "in" arguments/
public:
  using cxsom::update::Base::Base;

protected:

  // This is initialization, each time the update is onvoked.
  virtual void on_computation_start() override {
    if(!sum_out) sum_out = 0; // We reinitialize sum_out pnly if it has not been computed successfully yet.
    sum_in  = 0;
  }
  
  virtual void on_read_out_arg(const cxsom::symbol::Instance&, 
			       unsigned int,
			       const cxsom::data::Base& data) override {
    // We read the "out" argument and do our job with it/
    *sum_out += static_cast<const cxsom::data::d1::Pos&>(data).x;
  }
  
  virtual void on_read_out_arg_aborted() override {
    // Something went wrong (i.e some busy out argument has been
    // encountered). We clear the current sum_out computation. It will
    // be restarted from scratch at next update invocation.
    sum_out = std::nullopt;
  }
    
  virtual void on_read_in_arg(const cxsom::symbol::Instance&,
			      unsigned int,
			      const cxsom::data::Base& data) {
    sum_in += static_cast<const cxsom::data::d1::Pos&>(data).x;
  }
  
  virtual bool on_write_result(cxsom::data::Base& data) override {
    double& res = static_cast<cxsom::data::d1::Pos&>(data).x;
    double sum  = sum_in + *sum_out;
    if(sum == res)
      return false; // No actual update;
    res = sum;
    return true;
  }
};

#define CACHE_SIZE     5
#define FILE_SIZE     20
#define KEPT_OPENED true
int main(int argc, char* argv[]) {
  if(argc < 5) {
    std::cout << "Usage :" << std::endl
	      << "  " << argv[0] << " <timeline> <name> <at> set <value>" << std::endl
	      << "  " << argv[0] << " <timeline> <name> <at> get" << std::endl
	      << "  " << argv[0] << " <timeline> <name> <at> add <timeline> <name> <at> <timeline> <name> <at>" << std::endl
	      << std::endl
	      << "Hint : try the command" << std::endl
	      << "  cxsom-all-instances tmp/" << std::endl
	      << "after each call of " << argv[0] << '.' << std::endl;
    return 0;
  }

  cxsom::data::Center center(fs::current_path() / "tmp");

  std::string  timeline = argv[1];
  std::string  name     = argv[2];
  unsigned int at       = std::stoul(argv[3]);
  std::string  command  = argv[4];

  if(command == "get") {
    cxsom::symbol::Instance inst_sym {timeline, name, at};
    center.check(inst_sym, cxsom::type::make("Pos1D"), CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
 
    double value;
    center[inst_sym]->get([&value](auto, auto, auto& data) {
	value = static_cast<const cxsom::data::d1::Pos&>(data).x;
      });
    std::cout << inst_sym << " = " << value << std::endl;
  }
  
  else if(command == "set") {
    if(argc != 6) {
      std::cout << "Usage :" << std::endl
		<< "  " << argv[0] << " <timeline> <name> <at> set <value>" << std::endl;
      return 0;
    }
    cxsom::symbol::Instance inst_sym {timeline, name, at};
    center.check(inst_sym, cxsom::type::make("Pos1D"), CACHE_SIZE, FILE_SIZE, KEPT_OPENED);
    center[inst_sym]->set([val = std::stod(argv[5])](auto& status, auto&, auto& data) {
	if(status == cxsom::data::Availability::Ready) {
	  std::cout << "Instance already has a value." << std::endl;
	  return;	  
	}		  
	static_cast<cxsom::data::d1::Pos&>(data).x = val;
	status = cxsom::data::Availability::Ready;
      });
  }

  else if(command == "add") {
    if(argc != 11) {
      std::cout << "Usage :" << std::endl
  		<< "  " << argv[0] << " <timeline> <name> <at> add <timeline> <name> <at> <timeline> <name> <at>" << std::endl;
      return 0;
    }

    cxsom::symbol::Instance res {timeline, name, at};
    cxsom::symbol::Instance op1 {argv[5], argv[6], (unsigned int)std::stoul(argv[ 7])};
    cxsom::symbol::Instance op2 {argv[8], argv[9], (unsigned int)std::stoul(argv[10])};
    Addition add(center,
  		 {res, cxsom::type::make("Pos1D")},
  		 {cxsom::update::arg(op1, cxsom::type::make("Pos1D")),
  		  {op2, cxsom::type::make("Pos1D")}});
    auto status = add();
    std::cout << "Addition execution status = " << status << std::endl;
  }
  
  return 0;
}


