#include "logging.h"
#include "parameters.h"

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
			if (i != 11)
				sigaction(i, &act, nullptr);
			i++;
		}
	};
}

int main(int argc, char * args[])
{
	tissuestack::TissueStackConfigurationParameters * Params =
		tissuestack::TissueStackConfigurationParameters::instance();

	// we have been given the location of a startup configuration file
	if (argc > 1)
	{
		const std::string startUpConfFile = tissuestack::utils::Misc::convertCharPointerToString(args[1]);
		if (!tissuestack::utils::System::fileExists(startUpConfFile))
		{
			std::cerr << "\nERROR: Parameter given does not point to a file location!\n\n" <<
				"Usage: TissueStackImageServer [configuration file]\n" <<
				"Note: The configuration file (if used) has to be a text file with one key/value pair per line.\n" <<
				"Tabs are not appreciated and comment lines start with #. e.g:\n\n" <<
				"\t# TissueStack Image Server Port\n\tport=4242\n" <<
				"\t# Configuration database host\n\tdb_host=localhost\n" <<
				"\t# Configuration database port\n\tdb_port=5432\n" <<
				"\t# Configuration database name\n\tdb_name=tissuestack\n" <<
				"\t# Configuration database user\n\tdb_user=tissuestack\n" <<
				"\t# Configuration database password\n\tdb_password=tissuestack\n\n" << std::endl;
			Params->purgeInstance();
			exit(-1);
		}

		try
		{
			Params->readInConfigurationFile(startUpConfFile);
			Params->dumpConfigurationToDebugLog();
		} catch(std::exception & parseError)
		{
			std::cerr << "Failed to read passed in configuration file: " <<
					parseError.what() << std::endl;
			Params->purgeInstance();
			exit(-1);
		}
	}

	std::unique_ptr<tissuestack::networking::Server<tissuestack::common::TissueStackProcessingStrategy> >
				TissueStackServer;

	tissuestack::logging::TissueStackLogger * Logger = nullptr;

	try
	{
		Logger = tissuestack::logging::TissueStackLogger::instance();
		// create an instance of a tissue stack server and wrap it in a smart pointer
		TissueStackServer.reset(
				new tissuestack::networking::Server<tissuestack::common::TissueStackProcessingStrategy>(
						strtoul(Params->getParameter("port").c_str() , NULL, 10)));
	} catch (tissuestack::common::TissueStackException& ex) {
		tissuestack::logging::TissueStackLogger::instance()->error(
				"Failed to instantiate TissueStackServer with Default Strategy for the following reason: %s\n",
				ex.what());
		Params->purgeInstance();
		Logger->purgeInstance();
		exit(-1);
	} catch (std::exception & bad)
	{
		Logger->error(
				"Failed to instantiate TissueStackServer with Default Strategy for unexpected reason:\n%s\n", bad.what());
		Params->purgeInstance();
		Logger->purgeInstance();
		exit(-1);
	}

	try
	{
		// install the signal handler
		install_signal_handler(TissueStackServer.get());
	} catch (std::exception & bad)
	{
		Logger->error("Failed to install the signal handlers:\n%s\n", bad.what());
		Params->purgeInstance();
		Logger->purgeInstance();
		exit(-1);
	}

	try
	{
		// start the server socket
		TissueStackServer->start();
	} catch (tissuestack::common::TissueStackServerException& ex) {
		Logger->error(
				"Failed to start the TissueStack SocketServer for the following reason: %s\n",
				ex.what());
		Params->purgeInstance();
		Logger->purgeInstance();
		exit(-1);
	} catch (std::exception & bad)
	{
		Logger->error(
				"Failed to start the TissueStack SocketServer for unexpected reason:\n%s\n", bad.what());
		Params->purgeInstance();
		Logger->purgeInstance();
		exit(-1);
	}

	try
	{
		// start database connection
		if (!tissuestack::database::TissueStackPostgresConnector::instance()->isConnected())
		{
			Logger->error("Database is not connected!\n");
			tissuestack::database::TissueStackPostgresConnector::instance()->purgeInstance();
			Logger->purgeInstance();
			exit(-1);
		}
		Logger->info("Database connection established!\n");
		// TODO: move commented part into 'rest' section. also solidify with catch and tests for database connectivity
		/*
		const std::vector<tissuestack::database::Configuration *> conf =
				tissuestack::database::ConfigurationDataProvider::queryAllConfigurations();
		for (tissuestack::database::Configuration * c : conf)
		{
			Logger->debug("%s\n", c->getJson().c_str());
			delete c;
		}
		const tissuestack::database::Configuration * conf =
				tissuestack::database::ConfigurationDataProvider::queryConfigurationById("bla");
		if (conf)
		{
			Logger->debug("%s\n", conf->getJson().c_str());
			delete conf;
		}
		*/
	} catch (std::exception & bad)
	{
		Logger->error("Could not create databases connection:\n%s\n", bad.what());
		Params->purgeInstance();
		Logger->purgeInstance();
		exit(-1);
	}

	// instantiate singletons
	try
	{
		tissuestack::common::RequestTimeStampStore::instance(); // for request time stamp checking
	} catch (std::exception & bad)
	{
		Logger->error("Could not instantiate RequestTimeStampStore:\n%s\n", bad.what());
		Params->purgeInstance();
		Logger->purgeInstance();
		exit(-1);
	}

	try
	{
		tissuestack::imaging::TissueStackDataSetStore::instance(); // the data set store
		//tissuestack::imaging::TissueStackDataSetStore::instance()->dumpDataSetStoreIntoDebugLog();
	} catch (std::exception & bad)
	{
		tissuestack::common::RequestTimeStampStore::instance()->purgeInstance();
		Logger->error("Could not instantiate TissueStackDataSetStore:\n%s\n", bad.what());
		Params->purgeInstance();
		Logger->purgeInstance();
		exit(-1);
	}

	try
	{
		tissuestack::imaging::TissueStackLabelLookupStore::instance(); // for label lookups
		//tissuestack::imaging::TissueStackLabelLookupStore::instance()->dumpAllLabelLookupsToDebugLog();
	} catch (std::exception & bad)
	{
		tissuestack::imaging::TissueStackDataSetStore::instance()->purgeInstance();
		tissuestack::common::RequestTimeStampStore::instance()->purgeInstance();
		Logger->error("Could not instantiate TissueStackLabelLookupStore:\n%s\n", bad.what());
		Params->purgeInstance();
		Logger->purgeInstance();
		exit(-1);
	}

	try
	{
		tissuestack::imaging::TissueStackColorMapStore::instance(); // the colormap store
		//tissuestack::imaging::TissueStackColorMapStore::instance()->dumpAllColorMapsToDebugLog();
	} catch (std::exception & bad)
	{
		tissuestack::imaging::TissueStackLabelLookupStore::instance()->purgeInstance();
		tissuestack::imaging::TissueStackDataSetStore::instance()->purgeInstance();
		tissuestack::common::RequestTimeStampStore::instance()->purgeInstance();
		Logger->error("Could not instantiate TissueStackLabelLookupStore:\n%s\n", bad.what());
		Params->purgeInstance();
		Logger->purgeInstance();
		exit(-1);
	}

	try
	{
		InitializeMagick(NULL);
		// accept requests and process them until we receive a SIGSTOP
		TissueStackServer->listen();
	} catch (std::exception & bad)
	{
		if (!TissueStackServer->isStopping())
			Logger->error("TissueStackServer listen() was aborted for the following reason:\n%s\n", bad.what());
		TissueStackServer->stop();
		exit(-1);
	}
}
