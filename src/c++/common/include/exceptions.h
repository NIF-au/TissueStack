#ifndef	__EXCEPTIONS_H__
#define __EXCEPTIONS_H__

#include <exception>
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

    class TissueStackNullPointerException : public TissueStackException
    {
    	public:
			TissueStackNullPointerException();
			TissueStackNullPointerException(std::string what);
    };
  }
}

#endif	/* __EXCEPTIONS_H__ */
