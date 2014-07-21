#ifndef	__LOGGING_H__
#define __LOGGING_H__

// GLOBAL APPLICATION (DATA) PATH
#ifndef APPLICATION_PATH
#define APPLICATION_PATH "/usr/local/tissuestack"
#endif
// HELPS US TO ASSEMBLE SUB-DIRECTORIES BASED ON THE ROOT PATH
#define CONCAT_APP_PATH(PATH_TO_BE_ADDED) APPLICATION_PATH "/" PATH_TO_BE_ADDED

#include "utils.h"
#include "exceptions.h"
#include <mutex>
#include <cstdarg>

namespace tissuestack
{
  namespace logging
  {
    class TissueStackLogger final
    {
    	private:
    		const static std::string TIMESTAMP_FORMAT;
    		const std::string _log_path;
    		FILE * _info_log;
    		FILE * _error_log;
    		FILE * _debug_log;
    		TissueStackLogger(); /* : _log_path(CONCAT_APP_PATH("logs/"))
    		{
    			// check if path exists and create it if necessary
    			if (!tissuestack::utils::System::fileExists(this->_log_path))
    				if (!tissuestack::utils::System::createDirectory(this->_log_path, 0775))
    					THROW_TS_EXCEPTION(tissuestack::common::TissueStackServerException, "Unable to create the log path!");

    			// create and open files
    			this->_info_log = fopen(std::string(this->_log_path + "info.log").c_str(), "a");
    			this->_error_log = fopen(std::string(this->_log_path + "error.log").c_str(), "a");
    			this->_debug_log = fopen(std::string(this->_log_path + "debug.log").c_str(), "a");
    			if (this->_info_log == NULL || this->_error_log == NULL || this->_debug_log == NULL)
    				THROW_TS_EXCEPTION(tissuestack::common::TissueStackServerException, "Unable to create the log files");

    			this->all("TissueStackLogger initialized\n");
    		};*/

       		void log(FILE * log_file, const char * log_args, va_list args);
       		/*
			{
       			std::lock_guard<std::mutex> lock(this->_log_mutex);

				std::string timestamp = tissuestack::utils::System::getSystemTimeFormatted(TIMESTAMP_FORMAT);
				fprintf(log_file, "[%s]\t ", timestamp.c_str());

				vfprintf(log_file, log_args, args);
				fflush(log_file);
			}*/

    		static TissueStackLogger * _instance;
    		std::mutex _log_mutex;
    	public:
 			~TissueStackLogger();
 			/*
 			{
 				// close file handles
 				fclose(this->_info_log);
 				fclose(this->_error_log);
 				fclose(this->_debug_log);
 			};*/

    		static TissueStackLogger * instance();
    		/*
    		{
    			if (TissueStackLogger::_instance == nullptr)
    				TissueStackLogger::_instance = new TissueStackLogger();

    			return TissueStackLogger::_instance;
    		}*/

    		TissueStackLogger & operator=(const TissueStackLogger&) = delete;
    		TissueStackLogger(const TissueStackLogger&) = delete;

    		void info(const char * log_args, ...);
    		/*
    		{
				va_list args;
				va_start(args, log_args);
				this->log(this->_info_log, log_args, args);
				va_end(args);
    		}*/

    		void error(const char * log_args, ...);
    		/*
			{
				va_list args;
				va_start(args, log_args);
				this->log(this->_error_log, log_args, args);
				va_end(args);
			}*/

    		void debug(const char * log_args, ...);
    		/*
			{
				va_list args;
				va_start(args, log_args);
				this->log(this->_debug_log, log_args, args);
				va_end(args);
			}*/

    		void all(const std::string & log_line);
    		/*
    		{
    			this->info(log_line.c_str());
    			this->error(log_line.c_str());
    			this->debug(log_line.c_str());
    		}*/
    };
   }
}

#endif	/* __LOGGING_H__ */
