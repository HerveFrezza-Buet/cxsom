#include <cxsom-viewer.hpp>

#include <string>
#include <iostream>


int main(int argc, char* argv[]) {
  if(argc < 3) {
    std::cout << "Usage : " << argv[0] << " <rootdir> <timeline>" << std::endl
	      << std::endl;
    return 0;
  }

  std::string root_dir {argv[1]};
  std::string timeline {argv[2]};
  
  cxsom::symbol::Variable bmu {timeline, std::string("bmu")};
  cxsom::data::File file(root_dir, bmu);

  
  file.realize(nullptr, std::nullopt, std::nullopt, false);
  if(!file) {// We check that the file is realized.
    std::cerr << "The file is not realized, but no exception was thrown previously. Should not happen." << std::endl;
    return 1;
  }

  auto [tstart, tend] = file.get_time_range();
  std::cout << tstart << ' ' << tend << std::endl;
      
  return 0;
}
