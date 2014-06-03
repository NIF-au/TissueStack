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
		std::cout << "Received Signal : " << sig;

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
		//std::unique_ptr<tissuestack::networking::Server<tissuestack::common::TissueStackProcessingStrategy> >
			TissueStackServer.reset(new tissuestack::networking::Server<tissuestack::common::TissueStackProcessingStrategy>(4242));
	} catch (tissuestack::common::TissueStackException& ex) {
		std::cerr << "Failed to instantiate TissueStackServer with Default Strategy for the following reason:" << std::endl;
		std::cerr << ex.what() << std::endl;
		return -1;
	} catch (...)
	{
		std::cerr << "Failed to instantiate TissueStackServer with Default Strategy for unexpected reason!" << std::endl;
		return -1;
	}

	try
	{
		// install the signal handler
		install_signal_handler(TissueStackServer.get());
	} catch (...)
	{
		std::cerr << "Failed to install the signal handlers!" << std::endl;
		return -1;
	}

	std::unique_ptr<tissuestack::common::RequestTimeStampStore> TissueStackTimeStampStore;
	try
	{
		// instantiate TimeStampHashMap Singleton
		TissueStackTimeStampStore.reset(tissuestack::common::RequestTimeStampStore::instance());
	} catch (...)
	{
		std::cerr << "Failed to instantiate the RequestTimeStampStore!" << std::endl;
		return -1;
	}


	// instantiate a data set store singleton
	// TODO: implement

	// instantiate a color map store singleton
	// TODO: implement

	try
	{
		// start the server socket
		TissueStackServer->start();
	} catch (tissuestack::common::TissueStackServerException& ex) {
		std::cerr << "Failed to start the TissueStack SocketServer for the following reason:" << std::endl;
		std::cerr << ex.what() << std::endl;
		return -1;
	} catch (...)
	{
		std::cerr << "Failed to start the TissueStack SocketServer for unexpected reason!" << std::endl;
		return -1;
	}

	try
	{
		// accept requests and process them until we receive a SIGSTOP
		TissueStackServer->listen();
	} catch (tissuestack::common::TissueStackException& ex) {
		std::cerr << "TissueStackServer listen() was aborted for the following reason:" << std::endl;
		std::cerr << ex.what() << std::endl;
		return -1;
	} catch (...)
	{
		std::cerr << "TissueStackServer listen() was aborted for unexpected reason!" << std::endl;
		return -1;
	}

	// final cleaning up
	TissueStackServer->stop();

	return 1;
}
