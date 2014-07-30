#include "logging.h"
#include "singleton.h"

#include "server.h"
#include "networking.h"
#include "execution.h"
#include <memory>
#include <signal.h>

extern "C"
{
	// global pointer to give signal handler chance to call server stop
	tissuestack::networking::Server<tissuestack::common::TissueStackProcessingStrategy> * _ts_server_instance = nullptr;

	void handle_signals(int sig) {
		tissuestack::LoggerSingleton->info("Signal Handler Received Signal : %i\n", sig);

		switch (sig) {
			case SIGHUP:
			case SIGQUIT:
			case SIGTERM:
			case SIGINT:
			_ts_server_instance->stop();
			break;
		}
	};

	void install_signal_handler(tissuestack::networking::Server<tissuestack::common::TissueStackProcessingStrategy> * ts_server_instance)
	{
		if (ts_server_instance == nullptr) return;
		_ts_server_instance = ts_server_instance;

		struct sigaction act;
		int i;

		act.sa_handler = handle_signals;
		sigemptyset(&act.sa_mask);
		act.sa_flags = 0;
		i = 1;
		while (i < 32) {
			if (i != 11) // TODO: handle seg fault differently...
				sigaction(i, &act, nullptr);
			i++;
		}
	};
}

int main(int argc, char * args[])
{
	std::unique_ptr<tissuestack::networking::Server<tissuestack::common::TissueStackProcessingStrategy> >
				TissueStackServer;
	try
	{
		// create an instance of a tissue stack server and wrap it in a smart pointer
		TissueStackServer.reset(new tissuestack::networking::Server<tissuestack::common::TissueStackProcessingStrategy>(4242));
	} catch (tissuestack::common::TissueStackException& ex) {
		tissuestack::LoggerSingleton->error(
				"Failed to instantiate TissueStackServer with Default Strategy for the following reason: %s\n",
				ex.what());
		return -1;
	} catch (...)
	{
		tissuestack::LoggerSingleton->error(
				"Failed to instantiate TissueStackServer with Default Strategy for unexpected reason!\n");
		return -1;
	}

	try
	{
		// install the signal handler
		install_signal_handler(TissueStackServer.get());
	} catch (...)
	{
		tissuestack::logging::TissueStackLogger::instance()->error("Failed to install the signal handlers!\n");
		return -1;
	}

	try
	{
		// start the server socket
		TissueStackServer->start();
	} catch (tissuestack::common::TissueStackServerException& ex) {
		tissuestack::LoggerSingleton->error(
				"Failed to start the TissueStack SocketServer for the following reason: %s\n",
				ex.what());
		return -1;
	} catch (...)
	{
		tissuestack::LoggerSingleton->error(
				"Failed to start the TissueStack SocketServer for unexpected reason!\n");
		return -1;
	}


	try
	{
		// instantiate important singletons
		tissuestack::common::RequestTimeStampStore::instance(); // for request time stamp checking
		tissuestack::imaging::TissueStackDataSetStore::instance(); // the data set store
	// TODO: color map store
	} catch (...)
	{
		tissuestack::LoggerSingleton->error("Could not instantiate global singletons!\n");
		return -1;
	}

	try
	{
		// accept requests and process them until we receive a SIGSTOP
		TissueStackServer->listen();
	} catch (tissuestack::common::TissueStackException& ex) {
		if (!TissueStackServer->isStopping() && tissuestack::LoggerSingleton)
			tissuestack::LoggerSingleton->error(
					"TissueStackServer listen() was aborted for the following reason: %s\n",
					ex.what());
		return -1;
	} catch (...)
	{
		if (!TissueStackServer->isStopping() && tissuestack::LoggerSingleton)
			tissuestack::LoggerSingleton->error("TissueStackServer listen() was aborted for unexpected reason!\n");
		TissueStackServer->stop();
		return -1;
	}
}
