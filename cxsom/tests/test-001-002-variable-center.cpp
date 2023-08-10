#include <cxsom-server.hpp>

// This is used in case of some macros are set. You can remove this
// lines otherwise... or keep it.
cxsom::Tick*    cxsom::ticker  = new cxsom::Tick();
cxsom::Log*     cxsom::logger  = new cxsom::Log();
cxsom::Monitor* cxsom::monitor = new cxsom::Monitor();

#include <filesystem>
namespace fs = std::filesystem;



int main(int, char**) {
  cxsom::data::Center center(fs::current_path() / "my_center");

  try {
    center.check({"main", "/X/Bmu"}, cxsom::type::make("Pos1D"), 10, 10, false);
  } catch(std::exception& e) {std::cout << e.what() << std::endl;}
  
  try {
    center.check({"main", "/X/Bmu"}, cxsom::type::make("Dummy"));
  } catch(std::exception& e) {std::cout << e.what() << std::endl;}
  
  try {
    center.check({"main", "/X/Bmu"}, cxsom::type::make("Map1D<Scalar>=36"));
  } catch(std::exception& e) {std::cout << e.what() << std::endl;}
  
  try {
    center.check({"other", "/X/Bmu"}, cxsom::type::make("Map1D<Scalar>=36"), 10, 10, false);
  } catch(std::exception& e) {std::cout << e.what() << std::endl;}
  
  try {
    center.check({"other", "/X/Bmu"}, cxsom::type::make("Map1D<Scalar>=37"));
  } catch(std::exception& e) {std::cout << e.what() << std::endl;}

  std::cout << std::endl
	    << std::endl
	    << "Execute : find my_center" << std::endl
	    << std::endl;
  return 0;
}
