#include "system.h"

const unsigned int tissuestack::utils::System::getNumberOfCores()
{
	unsigned int cores = std::thread::hardware_concurrency();

	if (cores > 0) return static_cast<const unsigned int>(cores);
	
	return static_cast<const unsigned int>(sysconf( _SC_NPROCESSORS_ONLN ));
}

const unsigned long long int tissuestack::utils::System::getTotalRam()
{
  struct sysinfo memInfo;
  sysinfo (&memInfo);
  return static_cast<const unsigned long long int>(
      static_cast<unsigned long long int>(memInfo.totalram) * memInfo.mem_unit);
}

const unsigned long long int tissuestack::utils::System::getUsedRam()
{
  struct sysinfo memInfo;
  sysinfo (&memInfo);
  return static_cast<const unsigned long long int>(
      static_cast<unsigned long long int>(memInfo.freeram) * memInfo.mem_unit);
}

const unsigned long long int tissuestack::utils::System::getFreeRam()
{
  return static_cast<const unsigned long long int>(
     tissuestack::utils::System::getTotalRam() - tissuestack::utils::System::getUsedRam()
  );
}
