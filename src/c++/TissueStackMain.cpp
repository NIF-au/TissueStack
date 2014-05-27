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
	// create an instance of a tissue stack server and wrap it in a smart pointer
	std::unique_ptr<tissuestack::networking::Server<tissuestack::common::TissueStackProcessingStrategy> >
		TissueStackServer(new tissuestack::networking::Server<tissuestack::common::TissueStackProcessingStrategy>(4242));
	// install the signal handler
	install_signal_handler(TissueStackServer.get());

	// instantiate TimeStampHashMap Singleton
	// TODO: turn it into a singleton which it is not yet !
	//tissuestack::common::TimeStampHashMap::instance();

	// instantiate a data set store singleton
	// TODO: implement

	// instantiate a color map store singleton
	// TODO: implement

	// start the server socket
	TissueStackServer->start();

	// accept requests through the default TissueStackProcessingStrategy
	TissueStackServer->listen();

	// final cleaning up
	TissueStackServer->stop();
}
