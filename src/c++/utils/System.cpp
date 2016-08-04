/*
 * This file is part of TissueStack.
 *
 * TissueStack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TissueStack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TissueStack.  If not, see <http://www.gnu.org/licenses/>.
 */
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

const unsigned long long int tissuestack::utils::System::getFreeRam()
{
  struct sysinfo memInfo;
  sysinfo (&memInfo);
  return static_cast<const unsigned long long int>(
      static_cast<unsigned long long int>(memInfo.freeram) * memInfo.mem_unit);
}

const unsigned long long int tissuestack::utils::System::getUsedRam()
{
  return static_cast<const unsigned long long int>(
     tissuestack::utils::System::getTotalRam() - tissuestack::utils::System::getFreeRam()
  );
}

const bool tissuestack::utils::System::fileExists(const std::string& file_name)
{
	struct stat buffer;
	return (stat (file_name.c_str(), &buffer) == 0 && !S_ISDIR(buffer.st_mode));
}

const bool tissuestack::utils::System::directoryExists(const std::string& file_name)
{
	struct stat buffer;
	return (stat (file_name.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode));
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
		if (!tissuestack::utils::System::directoryExists(accumumatedDirectory))
			if (mkdir(accumumatedDirectory.c_str(), mode) < 0)
				return false;
	}

	return true;
}

const std::vector<std::string> tissuestack::utils::System::readTextFileLineByLine(const std::string & file)
{
	if (file.empty() || !tissuestack::utils::System::fileExists(file))
		return {};

	std::ifstream fileStream;
	fileStream.open(file.c_str(), std::ifstream::in);
	std::string line;
	std::vector<std::string> lines;
	while (std::getline(fileStream, line))
	{
		line = tissuestack::utils::Misc::eliminateWhitespaceAndUnwantedEscapeCharacters(line);
		if (line.empty() || line.at(0) == '#') continue;
		lines.push_back(line);
	}
	fileStream.close();

	return lines;
}

const unsigned long long int tissuestack::utils::System::getSystemTimeInMillis()
{
	auto time = std::chrono::system_clock::now();
	auto since_epoch = time.time_since_epoch();
	auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch);

	return static_cast<unsigned long long int>(millis.count());
}


const bool tissuestack::utils::System::touchFile(
	const std::string & file,
	const unsigned long long int size_in_bytes,
	const bool overwriteExistingFile)
{
	if (!overwriteExistingFile && tissuestack::utils::System::fileExists(file))
		return false;

	int fd = open(file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd <= 0)
		return false;

    if (size_in_bytes <= 0) {
        close(fd);
        return true;
    }

    if (ftruncate(fd, size_in_bytes) < 0) {
        close(fd);
		unlink(file.c_str());
		return false;
    }

    /*
    // fast forward and write 1 byte to cause padding!
	char buf[] = {'\0'};
    write(fd, (void *) buf, 1);
	if ((lseek(fd, size_in_bytes-1, SEEK_SET) < 0) ||
			write(fd, (void *) buf, 1) < 0)
	{
		close(fd);
		unlink(file.c_str());
		return false;
	}*/

	close(fd);

	return true;
}

const std::string tissuestack::utils::System::generateUUID() {
	uuid_t uuid;
	uuid_generate_time(uuid);

	std::stringstream stream;
	for(unsigned int i = 0; i < sizeof(uuid); i++)
		stream << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned short>(uuid[i]);

	return stream.str();
}

const std::string tissuestack::utils::System::generatePseudoRandomNumberAsString(const unsigned short digits)
{
	if (digits > 10) return "";

	srand (static_cast<unsigned int>(time(NULL)));
	unsigned long long int modulo = pow10(digits);
	std::string pseudoRandomNumber = std::to_string(rand () % modulo);

	for (unsigned short p=pseudoRandomNumber.length();p<digits;p++)
		pseudoRandomNumber.append("0");

	return pseudoRandomNumber;
}

const std::string tissuestack::utils::System::getSystemTimeFormatted(const std::string & format)
{
	 char buff[100];
	 time_t now = time (0);
	 strftime (buff, 100, format.c_str(), localtime (&now));

	return std::string(buff);
}

const time_t tissuestack::utils::System::getLastModifiedTime(const std::string & filename)
{
	if (!tissuestack::utils::System::fileExists(filename))
		return 0;

	struct stat buffer;
	stat(filename.c_str(), &buffer);
	return buffer.st_mtime;
}

const time_t tissuestack::utils::System::hasFileBeenModifiedSince(const std::string & filename, const time_t lastModification)
{
	const time_t latestModification = tissuestack::utils::System::getLastModifiedTime(filename);

	if (latestModification == 0)
			return 0;

	// return NULL if there hasn't been a modification (compared to the 2nd paramter time) or the new modification time if otherwise
	if (difftime(lastModification, latestModification) == 0)
		return 0;

	return latestModification;
}

const std::vector<std::string> tissuestack::utils::System::getFilesInDirectory(const std::string & directory)
{
	std::vector<std::string> files;

	if (!tissuestack::utils::System::directoryExists(directory))
		return files;

	DIR * dir = opendir(directory.c_str());
	struct dirent * dir_entry = NULL;

	if (dir == NULL)
		return files;

	while ((dir_entry = readdir(dir)))
	{
		if (!strcmp(dir_entry->d_name, ".") || !strcmp(dir_entry->d_name, ".."))
			continue;
		const std::string file = directory + "/" + dir_entry->d_name;
		if (tissuestack::utils::System::fileExists(file))
			files.push_back(file);
	}

	if (dir) closedir(dir);

	return files;
}

const bool tissuestack::utils::System::makeSocketNonBlocking(int socket_fd)
{
	int flags = fcntl(socket_fd, F_GETFL, 0);
	if (flags < 0)
		return false;

	fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
	if (flags < 0)
		return false;

	return true;
}

const unsigned long long int tissuestack::utils::System::getFileSizeInBytes(const std::string & file)
{
	if (!tissuestack::utils::System::fileExists(file))
		return 0;

	struct stat buf;

	if (stat(file.c_str(), &buf) == -1)
		return 0;

	return buf.st_size;
}

const unsigned long long int tissuestack::utils::System::getSpaceLeftGivenPathIntoPartition(const std::string & path)
{
	if (path.empty())
		return 0;

	struct statvfs buf;

	if (statvfs(path.c_str(), &buf) != 0)
		return 0;

	return static_cast<unsigned long long int>(buf.f_bavail) * buf.f_bsize;
}
