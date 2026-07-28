#include "app.hpp"

#include <iostream>

int main(int argc, char* argv[]) {
	if (symtool::app app(argc, argv); app) {
		app.run();
		std::cout << "Thanks for using symaths!" << std::endl;
		return 0;
	}

	std::cerr << "Error launching symaths command line tool." << std::endl;
	return 1;
}
