#ifndef	__COMMON_H__
#define __COMMON_H__

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
		};

		class RequestFilter
		{
			public:
				RequestFilter();
				virtual ~RequestFilter();
				virtual bool applyFilter(Request& request);
		};
	}
}

#endif	/* __COMMON_H__ */
