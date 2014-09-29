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
#ifndef	__EXCEPTIONS_H__
#define __EXCEPTIONS_H__

#include <exception>
#include <stdexcept>
#include <cstring>
#include <string>

//#define THROW_TS_EXCEPTION(ex, message) throw ex (message "[ " __FILE__ " @ " __LINE__ " ]")
#define EXPANDER(x) #x
#define EXPANDER2(x) EXPANDER(x)
#define THROW_TS_EXCEPTION(ex, message) throw ex ("ERROR: " message " [" __FILE__ "@ LINE: " EXPANDER2(__LINE__) "]");

namespace tissuestack
{
  namespace common
  {
    class TissueStackException : public std::exception
    {
    	public:
    		TissueStackException();
    		explicit TissueStackException(std::string what);
    		//void virtual setWhat(std::string what);
    		virtual const char * what() const throw();
    		//virtual ~TissueStackException() throw();
    	private:
    		std::string _what;
    };

    class TissueStackServerException : public TissueStackException
    {
    	public:
			TissueStackServerException();
			explicit TissueStackServerException(std::string what);
    };

    class TissueStackApplicationException : public TissueStackException
    {
    	public:
			explicit TissueStackApplicationException(std::string what);
    };

    class TissueStackFileUploadException : public TissueStackApplicationException
    {
    	public:
			explicit TissueStackFileUploadException(std::string what);
    };


    class TissueStackInvalidRequestException : public TissueStackApplicationException
    {
    	public:
			explicit TissueStackInvalidRequestException(std::string what);
    };

    class TissueStackObsoleteRequestException : public TissueStackInvalidRequestException
    {
    	public:
			explicit TissueStackObsoleteRequestException(std::string what);
    };

    class TissueStackNullPointerException : public TissueStackApplicationException
    {
    	public:
			explicit TissueStackNullPointerException(std::string what);
    };
  }
}

#endif	/* __EXCEPTIONS_H__ */
