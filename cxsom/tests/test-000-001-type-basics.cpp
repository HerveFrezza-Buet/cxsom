
#include <fstream>
#include <iostream>

#include <cxsom-server.hpp>

// This is used in case of some macros are set. You can remove this
// lines otherwise... or keep it.
cxsom::Tick* cxsom::ticker = new cxsom::Tick();
cxsom::Log*  cxsom::logger = new cxsom::Log();


// This gives a double insode the data, that we wil first write, and
// then read to check if everything is good.
double& hot_value(cxsom::data::Base& data);

int main(int, char**) {
  std::string type_buf;

  std::cout << "Enter a type description: " << std::flush;
  std::getline(std::cin, type_buf, '\n');

  auto type_ref = cxsom::type::make(type_buf);
  std::cout << std::endl
	    << "Type entered : " << type_ref->name() << '.' << std::endl
	    << "Length       : " << type_ref->byte_length() << " bytes." << std::endl
	    << std::endl;


  {
    auto data = cxsom::data::make(type_ref);
    hot_value(*data) = 3.1415;
    std::ofstream f("dump.data");
    data->write(f);
  }

  {
    auto data = cxsom::data::make(type_ref);
    std::ifstream f("dump.data");
    data->read(f);
    std::cout << "Hot value is " << hot_value(*data) << "... should be 3.1415." << std::endl
	      << std::endl;
  }
  
  
  return 0;
}

double& hot_value(cxsom::data::Base& data) {
  if(data.type->is_Scalar()) {
    auto& d = static_cast<cxsom::data::Scalar&>(data);
    return d.value;
  }
  if(data.type->is_Pos1D()) {
    auto& d = static_cast<cxsom::data::d1::Pos&>(data);
    return d.x;
  }
  if(data.type->is_Pos2D()) {
    auto& d = static_cast<cxsom::data::d2::Pos&>(data);
    return d.xy[0];
  }
  if(auto dim = data.type->is_Array()) {
    auto& d = static_cast<cxsom::data::Array&>(data);
    return d.content[dim / 2];
  }
  if(data.type->is_Map1D("Scalar")) {
    auto& d = static_cast<cxsom::data::Map&>(data);
    return d.content[0];
  }
  if(data.type->is_Map1D("Pos1D")) {
    auto& d = static_cast<cxsom::data::Map&>(data);
    return d.content[d.side - 1];
  }
  if(data.type->is_Map1D("Pos2D")) {
    auto& d = static_cast<cxsom::data::Map&>(data);
    unsigned int x = d.side / 2;
    unsigned int k = 0;
    return d.content[2 * x + k];
  }
  if(auto dim = data.type->is_MapOfArray(); data.type->is_Map1D() && dim > 0) {
    auto& d = static_cast<cxsom::data::Map&>(data);
    unsigned int x = d.side / 2;
    unsigned int k = dim    / 2;
    return d.content[dim * x  + k];
  }
  if(data.type->is_Map2D("Scalar")) {
    auto& d = static_cast<cxsom::data::Map&>(data);
    return d.content[0];
  }
  if(data.type->is_Map2D("Pos1D")) {
    auto& d = static_cast<cxsom::data::Map&>(data);
    return d.content[d.side * d.side - 1];
  }
  if(data.type->is_Map2D("Pos2D")) {
    auto& d = static_cast<cxsom::data::Map&>(data);
    unsigned int x = d.side / 2;
    unsigned int y = d.side / 2;
    unsigned int k = 0;
    return d.content[2 * (y * d.side + x) + k];
  }
  if(auto dim = data.type->is_MapOfArray(); data.type->is_Map2D() && dim > 0) {
    auto& d = static_cast<cxsom::data::Map&>(data);
    unsigned int x = d.side / 2;
    unsigned int y = d.side / 2;
    unsigned int k = dim    / 2;
    return d.content[dim * (y * d.side + x) + k];
  }
  else
    throw cxsom::error::unknown_type(data.type->name());
}

