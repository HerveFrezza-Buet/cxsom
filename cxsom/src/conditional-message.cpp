#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
  if(argc != 3)
    std::cout << "2 arguments required for conditional message." << std::flush;
  std::string word;
  std::cin >> word;
  if(word == "0")
    std::cout << argv[2] << std::flush;
  else
    std::cout << argv[1] << std::flush;
  return 0;
}
