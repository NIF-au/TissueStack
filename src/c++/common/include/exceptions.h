#ifndef	__EXCEPTIONS_H__
#define __EXCEPTIONS_H__

#include <exception>
#include <cstring>
#include <string>

namespace tissuestack
{
  namespace common
  {
    class TissueStackException final : public std::exception
    {
    	public:
    		TissueStackException();
    		TissueStackException(std::string what);
    		virtual const char * what() const throw();
    		~TissueStackException() throw();
    	private:
    		char * _what;
    };
  }

}

#endif	/* __EXCEPTIONS_H__ */
