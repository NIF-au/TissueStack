#ifndef	__EXCEPTIONS_H__
#define __EXCEPTIONS_H__

#include <exception>
#include <cstring>
#include <string>

namespace tissuestack
{
  namespace common
  {
    class TissueStackException : public std::exception
    {
    	public:
    		TissueStackException();
    		TissueStackException(std::string what);
    		void virtual setWhat(std::string what);
    		virtual const char * what() const throw();
    		virtual ~TissueStackException() throw();
    	private:
    		char * _what;
    };

    class TissueStackServerException : public TissueStackException
    {
    	public:
			TissueStackServerException();
			TissueStackServerException(std::string what);
    };

    class TissueStackInvalidRequestException : public TissueStackException
    {
    	public:
			TissueStackInvalidRequestException();
			TissueStackInvalidRequestException(std::string what);
    };
  }

}

#endif	/* __EXCEPTIONS_H__ */
