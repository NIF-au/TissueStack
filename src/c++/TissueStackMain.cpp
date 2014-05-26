#include "networking.h"
#include "execution.h"
#include <memory>

int main(int argc, char * args[])
{
	// create an instance of a tissue stack server
	tissuestack::networking::Server TissueStackServer(4242);
	// create server socket for listening
	TissueStackServer.start();

	// instantiate TimeStampHashMap Singleton
	// TODO: turn it into a singleton which it is not yet !
	//tissuestack::common::TimeStampHashMap::instance();

	// instantiate a data set store singleton
	// TODO: implement

	// instantiate a color map store singleton
	// TODO: implement

	/* Create a request processor for the specific needs of TissueStack:
	 *
	 * Looking at the runtime system we choose -depending on the number of cores-
	 * the size of the thread pool.
	 * In the sad case of a 1 core machine we stick to a sequential execution
	 */
	  tissuestack::common::TissueStackProcessingStrategy * TissueStackProcessingStrategy =
			  new tissuestack::common::TissueStackProcessingStrategy();
	  std::unique_ptr<const tissuestack::common::RequestProcessor<tissuestack::common::TissueStackProcessingStrategy> >
	  	  TissueStackProcessor(tissuestack::common::RequestProcessor<tissuestack::common::TissueStackProcessingStrategy>::instance(TissueStackProcessingStrategy));

	  // TODO: install the signal handler for both controlled shutdown and seg faults

	  /*
	   * We distribute the listening across the cores (if more than 1)
	   * This call will 'stall' the execution of main at any rate
	   * so that we use the signal handler to stop the listening
	   */
	  TissueStackServer.listen(TissueStackProcessor.get());

	  // final cleaning up
	  TissueStackServer.stop();
}
