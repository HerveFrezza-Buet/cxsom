#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
  std::cout << argv[1] << " (yes) : " << std::flush;
  std::string answer;
  std::cin >> answer;
  return answer == "yes";
}
