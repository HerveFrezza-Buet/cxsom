#include <cxsom-server.hpp>

#include <stdexcept>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

// In this example, we will play with the cxsom interface for handling
// the data stored in the files by a simulation. You may also perform
// such manipulation using the python library pycxsom.

#define ROOT_DIR "root_dir"

int main(int argc, char* argv[]) {
  if(argc < 4) {
    std::cout << "Usage : " << argv[0] << " <timeline> <varname> <command> [args...]" << std::endl
	      << "  Commands : " << std::endl
	      << "    realize <type> <cache_size> <file_size> // Realizes/creates the variable." << std::endl
	      << "    ++ <value>                              // Adds a next value."     << std::endl
	      << "    +@ <at> <value>                         // Adds a value at time <at>." << std::endl
	      << "    *@ <at>                                 // Reads the value at time <at>." << std::endl
	      << std::endl;
    return 0;
  }

  std::string command          {argv[3]};
  cxsom::symbol::Variable X    {std::string(argv[1]), argv[2]}; // This is our variable.
  cxsom::data::File       file {ROOT_DIR, X};                   // This is the file interface for X.
  int args = 4;
  int nb_command_args = argc - args;

  try {
    if(command == "realize") {
      if(nb_command_args != 3) {
	std::cerr << "Commande usage : realize <type> <cache_size> <file_size>" << std::endl;
	return 1;
      }
      // We use the arguments to link file to an actual file on the disk.
      auto type_ref = cxsom::type::make (argv[args + 0]);
      std::size_t cache_size = std::atoi(argv[args + 1]);
      std::size_t file_size  = std::atoi(argv[args + 2]);
      file.realize(type_ref, cache_size, file_size, false);
      if(!file) {// We check that the file is realized.
	std::cerr << "The file is not realized, but no exception was thrown previously. Should not happen." << std::endl;
	return 1;
      } 
    }
    else {
      
      file.realize(nullptr, std::nullopt, std::nullopt, false);
      if(!file) {// We check that the file is realized.
	std::cerr << "The file is not realized, but no exception was thrown previously. Should not happen." << std::endl;
	return 1;
      }

      // Let us build data for writing.
      auto data_ref = cxsom::data::make(file.get_type());
      
      if(command == "++") {
	if(nb_command_args != 1) {
	  std::cerr << "Commande usage : ++ <value>" << std::endl;
	  return 1;
	}
	*data_ref = std::atof(argv[args + 0]); // All elements of the data are set to the same value.
	auto at = file.get_next_time();
	std::cout << "Status = " << file.write(at, data_ref) << std::endl;
      }
      else if(command == "+@") {
	if(nb_command_args != 2) {
	  std::cerr << "Commande usage : +@ <at> <value>" << std::endl;
	  return 1;
	}
	std::size_t at = std::atoi(argv[args + 0]);
	*data_ref = std::atof(argv[args + 1]);
	std::cout << "Status = " << file.write(at, data_ref) << std::endl;
      }
      else if(command == "*@") {
	if(nb_command_args != 1) {
	  std::cerr << "Commande usage : *@ <at>" << std::endl;
	  return 1;
	}
	std::size_t at = std::atoi(argv[args + 0]);
	auto status = file.read(at, data_ref);
	std::cout << "Status = " << status << std::endl;
	if(status == cxsom::data::FileAvailability::Ready)
	  std::cout << "  got " << *data_ref << std::endl;
	else
	  std::cout << "  thus no reading was performed." << std::endl;
      }
      else
	std::cout << "Unhandled command \"" << command << "\". Nothing done except checking." << std::endl;
    }
  }
  catch(std::exception& e) {
    std::cout << std::endl << std::endl
	      << "Exception raised : " << e.what() << std::endl
	      << std::endl;
    return 1;
  }
  
  
  return 0;
}
