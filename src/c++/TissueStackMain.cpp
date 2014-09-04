#include "tissuestack.h"
#include "server.h"
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
	tissuestack::TissueStackConfigurationParameters * Params = nullptr;
	try
	{
		// instantiate default startup parameters
		Params = tissuestack::TissueStackConfigurationParameters::instance();
	} catch (std::exception & bad)
	{
		std::cerr << "Failed to instantiate default startup parameters!" << std::endl;
		exit(-1);
	}

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
			//Params->dumpConfigurationToDebugLog();
		} catch(std::exception & parseError)
		{
			std::cerr << "Failed to read passed in configuration file: " <<
					parseError.what() << std::endl;
			Params->purgeInstance();
			exit(-1);
		}
	}

	tissuestack::logging::TissueStackLogger * Logger = nullptr;
	try
	{
		// instantiate the logger
		Logger = tissuestack::logging::TissueStackLogger::instance();
	} catch (std::exception & bad)
	{
		Params->purgeInstance();
		std::cerr << "Failed to instantiate the logging mechanism!" << std::endl;
		exit(-1);
	}
	
	try
	{
		// start database connection
		if (!tissuestack::database::TissueStackPostgresConnector::instance()->isConnected())
		{
			Logger->error("Database is not connected!\n");
			tissuestack::database::TissueStackPostgresConnector::instance()->purgeInstance();
			Params->purgeInstance();
			Logger->purgeInstance();
			exit(-1);
		}
		Logger->info("Database connection established!\n");
		// clean up old sessions
		tissuestack::database::SessionDataProvider::deleteSessions(
			tissuestack::utils::System::getSystemTimeInMillis());
	} catch (std::exception & bad)
	{
		Logger->error("Could not create databases connection:\n%s\n", bad.what());
		Params->purgeInstance();
		Logger->purgeInstance();
		exit(-1);
	}

	try
	{
		tissuestack::common::RequestTimeStampStore::instance(); // for request time stamp checking
	} catch (std::exception & bad)
	{
		Logger->error("Could not instantiate RequestTimeStampStore:\n%s\n", bad.what());
		Params->purgeInstance();
		Logger->purgeInstance();
		tissuestack::database::TissueStackPostgresConnector::instance()->purgeInstance();
		exit(-1);
	}

	try
	{
		tissuestack::imaging::TissueStackLabelLookupStore::instance(); // for label lookups
		//tissuestack::imaging::TissueStackLabelLookupStore::instance()->dumpAllLabelLookupsToDebugLog();
	} catch (std::exception & bad)
	{
		Logger->error("Could not instantiate TissueStackLabelLookupStore:\n%s\n", bad.what());
		Params->purgeInstance();
		Logger->purgeInstance();
		tissuestack::database::TissueStackPostgresConnector::instance()->purgeInstance();
		tissuestack::common::RequestTimeStampStore::instance()->purgeInstance();
		exit(-1);
	}

	try
	{
		tissuestack::imaging::TissueStackColorMapStore::instance(); // the colormap store
		//tissuestack::imaging::TissueStackColorMapStore::instance()->dumpAllColorMapsToDebugLog();
	} catch (std::exception & bad)
	{
		Logger->error("Could not instantiate TissueStackLabelLookupStore:\n%s\n", bad.what());
		Params->purgeInstance();
		Logger->purgeInstance();
		tissuestack::database::TissueStackPostgresConnector::instance()->purgeInstance();
		tissuestack::common::RequestTimeStampStore::instance()->purgeInstance();
		tissuestack::imaging::TissueStackLabelLookupStore::instance()->purgeInstance();
		exit(-1);
	}

	try
	{
		tissuestack::imaging::TissueStackDataSetStore::instance(); // the data set store
		//tissuestack::imaging::TissueStackDataSetStore::instance()->dumpDataSetStoreIntoDebugLog();
	} catch (std::exception & bad)
	{
		Logger->error("Could not instantiate TissueStackDataSetStore:\n%s\n", bad.what());
		Params->purgeInstance();
		Logger->purgeInstance();
		tissuestack::database::TissueStackPostgresConnector::instance()->purgeInstance();
		tissuestack::common::RequestTimeStampStore::instance()->purgeInstance();
		tissuestack::imaging::TissueStackLabelLookupStore::instance()->purgeInstance();
		tissuestack::imaging::TissueStackColorMapStore::instance()->purgeInstance();
		exit(-1);
	}

	try
	{
		tissuestack::services::TissueStackTaskQueue::instance();
		//tissuestack::services::TissueStackTaskQueue::instance()->dumpAllTasksToDebugLog();
	} catch (std::exception & bad)
	{
		Logger->error("Could not instantiate TissueStackTaskQueue:\n%s\n", bad.what());
		Params->purgeInstance();
		Logger->purgeInstance();
		tissuestack::database::TissueStackPostgresConnector::instance()->purgeInstance();
		tissuestack::common::RequestTimeStampStore::instance()->purgeInstance();
		tissuestack::imaging::TissueStackLabelLookupStore::instance()->purgeInstance();
		tissuestack::imaging::TissueStackColorMapStore::instance()->purgeInstance();
		tissuestack::imaging::TissueStackDataSetStore::instance()->purgeInstance();
		exit(-1);
	}

	std::unique_ptr<tissuestack::networking::Server<tissuestack::common::TissueStackProcessingStrategy> >
				TissueStackServer;
	try
	{
		InitializeMagick(NULL);
		// create an instance of a tissue stack server and wrap it in a smart pointer
		TissueStackServer.reset(
				new tissuestack::networking::Server<tissuestack::common::TissueStackProcessingStrategy>(
						strtoul(Params->getParameter("port").c_str() , NULL, 10)));
		// start the server socket
		TissueStackServer->start();
	} catch (std::exception & bad)
	{
		Logger->error(
				"Failed to instantiate/start the TissueStackServer for the following reason:\n%s\n", bad.what());
		Params->purgeInstance();
		Logger->purgeInstance();
		tissuestack::database::TissueStackPostgresConnector::instance()->purgeInstance();
		tissuestack::common::RequestTimeStampStore::instance()->purgeInstance();
		tissuestack::imaging::TissueStackLabelLookupStore::instance()->purgeInstance();
		tissuestack::imaging::TissueStackColorMapStore::instance()->purgeInstance();
		tissuestack::imaging::TissueStackDataSetStore::instance()->purgeInstance();
		tissuestack::services::TissueStackTaskQueue::instance()->purgeInstance();
		exit(-1);
	}

	try
	{
		// install the signal handler
		install_signal_handler(TissueStackServer.get());
	} catch (std::exception & bad)
	{
		Logger->error("Failed to install signal handlers: %s!\n",bad.what());
		TissueStackServer->stop();
		exit(-1);
	}
		
	try
	{
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
