#include <iostream>

// This executable only displays help to the user when s/he tries the 005-001 examples.


int main(int, char*[]) {
  std::cout << std::endl << std::endl
	    << "Running example 005-001 (custom rules)"                                                             << std::endl
	    << "-----------------------"                                                                            << std::endl
	    << std::endl
	    << "  1 - create a <root-dir> directory"                                                                << std::endl
	    << "  2 - run in another terminal : cxsom-example-005-001-foo-processor <root-dir> <nb-threads> <port>" << std::endl
	    << "  3 - display rules :  cxsom-example-005-001-basic-test graph foo"                                  << std::endl
	    << "                       dot -Tpdf foo-updates.dot -o foo-updates.pdf"                                << std::endl
	    << "                       evince foo-updates.pdf"                                                      << std::endl
	    << "  4 - send rules :  cxsom-example-005-001-basic-test send localhost <port>"                         << std::endl
	    << "  5 - check evolution : cxsom-all-instances <root-dir>"                                             << std::endl
	    << "  6 - You can kill the process started at step 2 now."                                              << std::endl
	    << std::endl;

  return 0;
}
