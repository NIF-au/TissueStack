#include "networking.h"
#include <memory>

int main(int argc, char * args[])
{
  tissuestack::networking::Server server;

  try
  {
	  std::unique_ptr<tissuestack::utils::Timer> t(tissuestack::utils::Timer::getInstance(tissuestack::utils::Timer::Type::CLOCK_GET_TIME));
	  t->start();
	  server.start();
	  server.stop();
	  std::cout << "Time: " << static_cast<double>(t->stop()) << std::endl;

	  /*
	  std::vector<std::shared_ptr<tissuestack::common::RequestFilter> > filter_chain
	  {
		std::shared_ptr<tissuestack::common::RequestFilter>(new tissuestack::networking::HttpRequestSanityFilter())
	  };
	  tissuestack::networking::RawHttpRequest req(std::string("GET HTTP1.1 \n kakakakakakakak"));
	  for (auto filter : filter_chain)
		  if (!filter->applyFilter(req))
			throw tissuestack::common::TissueStackInvalidRequestException(std::string("request did not get past sanity check!"));
;

	  //tissuestack::networking::HttpRequest httpReq(req);
	   */

  }  catch (tissuestack::common::TissueStackInvalidRequestException& ex)
  {
	  std::cerr << ex.what() << std::endl;
	  return -1;
  }  catch (...)
  {
	  std::cerr << "Something bad happened" << std::endl;
	  return -1;
  }

  return 0;
}
