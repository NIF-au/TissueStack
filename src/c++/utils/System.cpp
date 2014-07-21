#include "utils.h"

tissuestack::utils::System::System() {}

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

const bool tissuestack::utils::System::fileExists(const std::string& file_name)
{
	struct stat buffer;
	return (stat (file_name.c_str(), &buffer) == 0);
}

const bool tissuestack::utils::System::createDirectory(const std::string& directory, mode_t mode)
{
	if (directory.empty())
		return true;

	// loop over tokens and see if they exist, if not, create
	std::vector<std::string> directories = tissuestack::utils::Misc::tokenizeString(directory, '/');
	std::string accumumatedDirectory = "";
	for (auto subdir : directories)
	{
		accumumatedDirectory += (subdir + "/");
		if (!tissuestack::utils::System::fileExists(accumumatedDirectory))
			if (mkdir(accumumatedDirectory.c_str(), mode) < 0) return false;
	}

	return true;
}

const std::string tissuestack::utils::System::getSystemTimeFormatted(const std::string & format)
{
	 char buff[100];
	 time_t now = time (0);
	 strftime (buff, 100, format.c_str(), localtime (&now));

	return std::string(buff);
}
