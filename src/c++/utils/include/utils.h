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
#ifndef	__SYSTEM_H__
#define __SYSTEM_H__

#include <thread>

#include <fstream>
#include <iostream>
#include <iomanip>

#include <fcntl.h>

#include <ctime>
#include <sstream>
#include <cstring>

#include <unistd.h>

#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>

#include <dirent.h>
#include <typeinfo>
#include <cxxabi.h>

#include <algorithm>

#include <vector>
#include <unordered_map>

#include <stdexcept>
#include <uuid/uuid.h>

#include <zlib.h>
#include <zip.h>
#include <sys/statvfs.h>

namespace tissuestack
{
  namespace utils
  {
    class System final
    {
      public:
        static const unsigned int getNumberOfCores();
        static const unsigned long long int getTotalRam();
        static const unsigned long long int getUsedRam();
        static const unsigned long long int getFreeRam();
        static const bool fileExists(const std::string& file_name);
        static const bool directoryExists(const std::string& file_name);
        static const bool createDirectory(const std::string& directory, mode_t mode);
        static const std::vector<std::string> readTextFileLineByLine(const std::string & file);
        static const std::string getSystemTimeFormatted(const std::string & format);
        static const time_t getLastModifiedTime(const std::string & filename);
        static const time_t hasFileBeenModifiedSince(const std::string & filename, const time_t lastModification);
        static const unsigned long long int getSystemTimeInMillis();
        static const bool touchFile(
        	const std::string & file,
        	const unsigned long long int size_in_bytes,
        	const bool overwriteExistingFile = false);
        static const std::string generateUUID();
        static const std::string generatePseudoRandomNumberAsString(const unsigned short digits);
        static const std::vector<std::string> getFilesInDirectory(const std::string & directory);
        static const bool makeSocketNonBlocking(int socket_fd);
        static const unsigned long long int getFileSizeInBytes(const std::string & file);
        static const unsigned long long int getSpaceLeftGivenPathIntoPartition(const std::string & path);
      private:
        System();
        System & operator=(const System&) = delete;
        System(const System&) = delete;
    };

    class Misc final
    {
      public:
    	static const std::string maskQuotesInJson(const std::string & json);
        static const std::string convertCharPointerToString(const char * some_characters);
    	static const std::string demangleTypeIdName(const char * mangeledIdName);
    	static const std::vector<std::string> tokenizeString(const std::string & some_string, const char delimiter);
    	static const std::string findUnorderedMapEntryWithUpperCaseStringKey(
    			const std::unordered_map<std::string, std::string> & map, std::string key);
    	static const std::string composeHttpResponse(
    			const std::string status,
    			const std::string content_type,
    			const std::string content,
    			const bool gzipped = false);
    	static const std::string sanitizeSqlQuote(const std::string & quoted_value);
    	static const std::string eraseCharacterFromString(const std::string & someString, const char unwantedCharacter);
    	static const std::string eliminateWhitespaceAndUnwantedEscapeCharacters(const std::string & someString);
    	static const bool streamGzippedDataToDescriptor(unsigned char * data, const unsigned int length, const int descriptor);
    	static const std::vector<std::string> getContentsOfZipArchive(const std::string & archive);
    	static const bool extractZippedFileFromArchive(
    		const std::string & archive,
			const std::string & file_to_be_extracted,
			const std::string & new_file_location,
			const bool & overwriteExistingFile = false);
      private:
    	Misc();
    	Misc & operator=(const Misc&) = delete;
    	Misc(const Misc&) = delete;
    };

    class Timer
      {
        public:
          // NANO SECONDS CONVERSION
          static const unsigned long long int NANO_SECONDS_PER_SEC = 1000000000;

          // TYPES OF TIMERS
          enum class Type
          {
              CLOCK_GET_TIME,
              CLOCK_TICKS
          };

          // FACTORY METHOD
          static Timer * const getInstance(Type type_of_timer);

          // VIRTUAL TIMER METHODS
          virtual void start() = 0;
          virtual const unsigned long long int stop() = 0;
          virtual ~Timer(){};
        protected:
          Timer(){};
      };

      class OrdinaryTimer : public Timer
      {
        friend class Timer;

        public:
          void start();
          const unsigned long long int stop();

        private:
          struct timespec _clock_start;
          struct timespec _clock_end;
          OrdinaryTimer() {};
      };

      class ClockTimer : public Timer
      {
        friend class Timer;

        public:
          void start();
          const unsigned long long int stop();

        private:
          clock_t _clock_start;
          clock_t _clock_end;
          ClockTimer() : _clock_start(0), _clock_end(0) {};
      };
    }
}

#endif	/* __SYSTEM_H__ */
