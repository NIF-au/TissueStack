#include <iostream>
#include <cstdlib>
#include <string.h>

extern "C" {
	int lala(const char * message)
	{
		if (message == nullptr) {
			std::cout << "You gave me NULL!" << std::endl;
			return 0;
		}

		// lets be evil
		char * message_writable = (char *) message;
		int l = strlen(message);
		while (l >= 0)
		{
			message_writable[l] = 'x';
			l--;
		}
		return -1;
	}
}
