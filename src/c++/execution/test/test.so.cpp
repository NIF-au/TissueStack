#include <iostream>
#include <cstdlib>

extern "C" {
	int lala(const char * message)
	{
		if (message == nullptr) {
			std::cout << "You gave me NULL!" << std::endl;
			return 0;
		}
		std::cout << message << std::endl;
		return -1;
	}
}
