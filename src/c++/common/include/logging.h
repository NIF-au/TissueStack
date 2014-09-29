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
#ifndef	__LOGGING_H__
#define __LOGGING_H__

#include "globals.h"
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
 			static const bool doesInstanceExist();
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
