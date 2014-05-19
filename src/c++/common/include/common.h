#ifndef	__COMMON_H__
#define __COMMON_H__

#include <string>

namespace tissuestack
{
	namespace common
	{
		class RequestProcessor
		{
			public:
				RequestProcessor();
				virtual ~RequestProcessor();
				//virtual void processRequest(std::function) = 0;
		};

		class Request
		{
			protected:
				Request();
			public:
				virtual const std::string getContent() const = 0;
				virtual ~Request();
		};

		class RequestFilter
		{
			protected:
				RequestFilter();
			public:
				virtual ~RequestFilter();
				virtual const bool applyFilter(Request & in) const = 0;
		};
	}
}

#endif	/* __COMMON_H__ */
