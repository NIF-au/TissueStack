#include "networking.h"

int main(int argc, char * args[])
{
  tissuestack::networking::Server server;

  try
  {
	  tissuestack::utils::Timer * t = tissuestack::utils::Timer::getInstance(tissuestack::utils::Timer::Type::CLOCK_GET_TIME);
	  t->start();
	  server.start();
	  server.stop();
	  t->stop();
  }  catch (...) {
	  return 1;
  }

  return 0;
}
