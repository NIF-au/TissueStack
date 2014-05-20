#include "networking.h"

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

	  // NOTE: try templated RequestFilter with in and out type handed in: applyFilter takes in and spits out out
	  std::vector<std::shared_ptr<tissuestack::common::RequestFilter> > filter_chain
	  {
		  std::shared_ptr<tissuestack::common::RequestFilter>(new tissuestack::networking::HttpRequestSanityFilter())
	  };

	  std::unique_ptr<std::string> someInput(new std::string("GET HTTP1.1 \n kakakakakakakak"));
	  std::unique_ptr<const tissuestack::common::Request> req(new tissuestack::networking::RawHttpRequest(someInput.get()));
	  for (auto filter : filter_chain)
		  req.reset(filter->applyFilter(req.get()));

	  // TODO: nullptr checks and devise good handing in by reference/smart pointer practices

		  //std::cout << req->getContent() << std:: endl;

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
