#include "utils.h"

int main(int argc, char * args[])
{
  /* cursory timer tests */
  unsigned long long int d = 0;
  
  tissuestack::utils::Timer * t = tissuestack::utils::Timer::getInstance(tissuestack::utils::Timer::Type::CLOCK_GET_TIME);
  t->start();
  sleep(2);
  d = t->stop() / tissuestack::utils::Timer::NANO_SECONDS_PER_SEC;
  if (d != 2) return 0;  
  delete t;

  t = tissuestack::utils::Timer::getInstance(tissuestack::utils::Timer::Type::CLOCK_TICKS);
  t->start();
  sleep(2);
  d = t->stop() / tissuestack::utils::Timer::NANO_SECONDS_PER_SEC;
  if (d != 0) return 0;  

  return 1;
}
