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
    		TissueStackLogger();
       		void log(FILE * log_file, const char * log_args, va_list args);
      		static TissueStackLogger * _instance;
    		std::mutex _log_mutex;
    	public:
 			~TissueStackLogger();
    		static TissueStackLogger * instance();
    		static void purgeInstance();
     		TissueStackLogger & operator=(const TissueStackLogger&) = delete;
    		TissueStackLogger(const TissueStackLogger&) = delete;

    		void info(const char * log_args, ...);
    		void error(const char * log_args, ...);
    		void debug(const char * log_args, ...);
    		void all(const std::string & log_line);
    };
   }
}

#endif	/* __LOGGING_H__ */
