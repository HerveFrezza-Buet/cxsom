#include <cxsom-viewer.hpp>

#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>

#define MAP_SIZE 500
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

  std::vector<unsigned char> img(MAP_SIZE);
  std::fill(img.begin(), img.end(), (unsigned char)255);
  auto data_ref = cxsom::data::make(file.get_type());
  for(auto t = tstart; t <= tend; ++t) {
    file.read(t, data_ref);
    double at = static_cast<cxsom::data::Scalar&>(*data_ref).value;
    unsigned int pos = (unsigned int)((MAP_SIZE - 1)*at +.5);
    img[pos] = (unsigned char)0;
  }

  {
    std::ofstream ppm {timeline + "-bmus.ppm"};
    ppm << "P5\n" << MAP_SIZE << " 1\n255\n";
    ppm.write((char*)std::data(img), MAP_SIZE);
  }
      
  return 0;
}
