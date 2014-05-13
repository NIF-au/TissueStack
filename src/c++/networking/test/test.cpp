#include "server.h"

int main(int argc, char * args[])
{
  tissuestack::networking::Server server;

  try
  {
	  server.start();
	  server.stop();
  } catch (std::exception& e)
  {
	  std::cerr << e.what() << std::endl;
	  return 1;
  }

  return 0;
}
