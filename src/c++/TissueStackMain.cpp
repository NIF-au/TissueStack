/*
 * This file is part of TissueStack.
 *
 * TissueStack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TissueStack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TissueStack.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "tissuestack.h"
#include "server.h"
#include <signal.h>


void cleanUp()
{
	try
	{
		// clean up old sessions and disconnect database
		tissuestack::database::SessionDataProvider::deleteSessions(
			tissuestack::utils::System::getSystemTimeInMillis());

		if (tissuestack::database::TissueStackPostgresConnector::doesInstanceExist())
			tissuestack::database::TissueStackPostgresConnector::instance()->purgeInstance();
	} catch (...)
	{
		// can be safely ignored
	}

	try
	{
		// deallocate global singleton objects
		if (tissuestack::TissueStackConfigurationParameters::doesInstanceExist())
			tissuestack::TissueStackConfigurationParameters::instance()->purgeInstance();

		if (tissuestack::common::RequestTimeStampStore::doesInstanceExist())
			tissuestack::common::RequestTimeStampStore::instance()->purgeInstance();

		if (tissuestack::imaging::TissueStackDataSetStore::doesInstanceExist())
			tissuestack::imaging::TissueStackDataSetStore::instance()->purgeInstance();

		if (tissuestack::imaging::TissueStackLabelLookupStore::doesInstanceExist())
			tissuestack::imaging::TissueStackLabelLookupStore::instance()->purgeInstance();

		if (tissuestack::imaging::TissueStackColorMapStore::doesInstanceExist())
			tissuestack::imaging::TissueStackColorMapStore::instance()->purgeInstance();

		if (tissuestack::services::TissueStackTaskQueue::doesInstanceExist())
			tissuestack::services::TissueStackTaskQueue::instance()->purgeInstance();

		if (tissuestack::logging::TissueStackLogger::doesInstanceExist())
			tissuestack::logging::TissueStackLogger::instance()->purgeInstance();

	} catch (...)
	{
		// can be safely ignored
	}
	DestroyMagick();
}

// global pointer to give signal handler chance to call server stop
std::unique_ptr<tissuestack::networking::Server<tissuestack::common::TissueStackProcessingStrategy> >
			TissueStackServer;

void handle_signals(int sig) {
	switch (sig) {
		case SIGHUP:
		case SIGQUIT:
		case SIGTERM:
		case SIGINT:
			if (TissueStackServer) TissueStackServer->stop();
			cleanUp();
			break;
	}
};

void install_signal_handler()
{
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


int main(int argc, char * args[])
{
	try
	{
		// install the signal handler
		install_signal_handler();
	} catch (std::exception & bad)
	{
		std::cerr << "Failed to install signal handlers!" << std::endl;
		exit(-1);
	}

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
			//Params->purgeInstance();
			cleanUp();
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
			//Params->purgeInstance();
			cleanUp();
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
		//Params->purgeInstance();
		cleanUp();
		std::cerr << "Failed to instantiate the logging mechanism!" << std::endl;
		exit(-1);
	}
	
	try
	{
		// start database connection
		if (!tissuestack::database::TissueStackPostgresConnector::instance()->isTransConnected()
				|| !tissuestack::database::TissueStackPostgresConnector::instance()->isNonTransConnected(0)
				|| !tissuestack::database::TissueStackPostgresConnector::instance()->isNonTransBackupConnected())
		{
			Logger->error("Database is not connected!\n");
			//tissuestack::database::TissueStackPostgresConnector::instance()->purgeInstance();
			//Params->purgeInstance();
			//Logger->purgeInstance();
			cleanUp();
			exit(-1);
		}
		Logger->info("Database connection established!\n");
		// clean up old sessions
		tissuestack::database::SessionDataProvider::deleteSessions(
			tissuestack::utils::System::getSystemTimeInMillis());
	} catch (std::exception & bad)
	{
		Logger->error("Could not create databases connection:\n%s\n", bad.what());
		cleanUp();
		exit(-1);
	}

	try
	{
		tissuestack::common::RequestTimeStampStore::instance(); // for request time stamp checking
	} catch (std::exception & bad)
	{
		Logger->error("Could not instantiate RequestTimeStampStore:\n%s\n", bad.what());
		cleanUp();
		//tissuestack::database::TissueStackPostgresConnector::instance()->purgeInstance();
		exit(-1);
	}

	try
	{
		tissuestack::imaging::TissueStackLabelLookupStore::instance(); // for label lookups
		//tissuestack::imaging::TissueStackLabelLookupStore::instance()->dumpAllLabelLookupsToDebugLog();
	} catch (std::exception & bad)
	{
		Logger->error("Could not instantiate TissueStackLabelLookupStore:\n%s\n", bad.what());
		cleanUp();
		exit(-1);
	}

	try
	{
		tissuestack::imaging::TissueStackColorMapStore::instance(); // the colormap store
		//tissuestack::imaging::TissueStackColorMapStore::instance()->dumpAllColorMapsToDebugLog();
	} catch (std::exception & bad)
	{
		Logger->error("Could not instantiate TissueStackLabelLookupStore:\n%s\n", bad.what());
		cleanUp();
		exit(-1);
	}

	try
	{
		tissuestack::imaging::TissueStackDataSetStore::instance(); // the data set store
		//tissuestack::imaging::TissueStackDataSetStore::instance()->dumpDataSetStoreIntoDebugLog();
	} catch (std::exception & bad)
	{
		Logger->error("Could not instantiate TissueStackDataSetStore:\n%s\n", bad.what());
		cleanUp();
		exit(-1);
	}

	try
	{
		tissuestack::services::TissueStackTaskQueue::instance();
		//tissuestack::services::TissueStackTaskQueue::instance()->dumpAllTasksToDebugLog();
	} catch (std::exception & bad)
	{
		Logger->error("Could not instantiate TissueStackTaskQueue:\n%s\n", bad.what());
		cleanUp();
		exit(-1);
	}

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
		cleanUp();
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
		cleanUp();
		exit(-1);
	}
}
