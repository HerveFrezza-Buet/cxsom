#include <cxsom-server.hpp>

// This is used in case of some macros are set. You can remove this
// lines otherwise... or keep it.
cxsom::Tick*    cxsom::ticker  = new cxsom::Tick();
cxsom::Log*     cxsom::logger  = new cxsom::Log();
cxsom::Monitor* cxsom::monitor = new cxsom::Monitor();

#include <filesystem>
namespace fs = std::filesystem;

#include <fstream>
#include <iostream>
#include <iomanip>

int main(int argc, char* argv[]) {
  if(argc == 1) {
    std::cout << "Usage :" << std::endl
	      << "  " << argv[0] << " init             <-- not mandatory, it creates the file an does nothing." << std::endl
	      << "  " << argv[0] << " show             <-- show the file content." << std::endl
	      << "  " << argv[0] << " set <at> <value> <-- adds an element." << std::endl;
    return 0;
  }
  std::string command(argv[1]);

  fs::path root_dir = fs::current_path();
  cxsom::symbol::Variable var_symb {"timeline", "foo"};
  auto type = cxsom::type::make("Pos1D");

  cxsom::data::Variable v(root_dir, var_symb, type,
			  1,      // cache_size
			  10,     // file buffer size
			  false); // We do not keep it opened.

  if(command == "show") {
    unsigned int nb = v.history_length();
    
    for(unsigned int at = 0; at < nb; ++at) {
      auto i_ref = v[at];
      i_ref->get([at](auto status, auto, auto& data) {
	  if(status == cxsom::data::Availability::Ready) std::cout << "\u2713";
	  else                                           std::cout <<      ' ';
	  std::cout << std::setw(3) << at 
		    << " : " << static_cast<const cxsom::data::d1::Pos&>(data).x
		    << std::endl;
	});
    }
    return 0;
  }

  if(command == "set") {
    if(argc != 4) {
      std::cout << "Usage :"  << std::endl
		<< "  " << argv[0] << " set <at> <value> <-- adds an element." << std::endl;
      return 0;
    }

    unsigned int  at = std::stoul(argv[2]);
    double       val = std::stod(argv[3]);
    auto i_ref = v[at];
    i_ref->set([val](auto& status, auto&, auto& data) {
	static_cast<cxsom::data::d1::Pos&>(data).x = val;
	status = cxsom::data::Availability::Ready;
      });
    return 0;
  }

  return 0;
}
