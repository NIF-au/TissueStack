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
#include "execution.h"
#include "networking.h"
#include "imaging.h"

#include <signal.h>
#include <getopt.h>

void cleanUp()
{
	try
	{
		if (tissuestack::imaging::TissueStackColorMapStore::doesInstanceExist())
			tissuestack::imaging::TissueStackColorMapStore::instance()->purgeInstance();

		if (tissuestack::imaging::TissueStackLabelLookupStore::doesInstanceExist())
			tissuestack::imaging::TissueStackLabelLookupStore::instance()->purgeInstance();
	} catch (...)
	{
		// can be safely ignored
	}
}

std::unique_ptr<tissuestack::execution::TissueStackOfflineExecutor> OfflineExecutor(
		tissuestack::execution::TissueStackOfflineExecutor::instance());

void handle_signals(int sig) {
	switch (sig) {
		case SIGHUP:
		case SIGQUIT:
		case SIGTERM:
		case SIGINT:
			std::cerr << "\nReceived Crtl + C!" << std::endl;
			OfflineExecutor->stop();
			std::cerr << "Waiting 5 seconds to abort tiling properly!" << std::endl;
			sleep(5);
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

int		main(int argc, char **argv)
{
	std::string color_map = "grey";
	std::string in_file = "";
	std::string tile_dir = "";
	std::string zoom_levels = "0,1,2,3,4,5,6,7";
	std::string dimensions = "x,y,z";

	int c = 0;
	while (1)
	{
		static struct option long_options[] = {
			{"dimension",	optional_argument, 0, 'd'},
			{"level",  		optional_argument, 0, 'l'},
			{"color",  		optional_argument, 0, 'c'},
			{"path",  		required_argument, 0, 'p'},
			{"file",    	required_argument, 0, 'f'},
			{0, 0, 0, 0}
		};

		int option_index = 0;
		c = getopt_long (argc, argv, "d:l:c:p:f:", long_options, &option_index);
		if (c == -1)
			break;

		const std::string tmp =
			(optarg == NULL) ? "" : std::string(optarg, strlen(optarg));

		switch (c)
		{
			case 'c':
				color_map = tmp;
				break;

			case 'd':
				dimensions = tmp;
				break;

			case 'l':
				zoom_levels = tmp;
				break;

			case 'p':
				tile_dir = tmp;
				break;

			case 'f':
				in_file = tmp;
				break;

			case '?':
				exit (0);   /* getopt_long already printed an error message. */
			break;

			default:
				std::cout << "Usage: " << argv[0] <<
					" -d DIMENSION_NAMES -l LEVELS [-c COLORMAP] -p PATH -f FILE\n";
			exit(0);
		}
	}

	// check for mandatory params
	if (in_file.empty() || tile_dir.empty())
	{
		std::cerr << "Usage: " << argv[0] <<
			" -d DIMENSION_NAMES -l LEVELS [-c COLORMAP] -p PATH -f FILE\n";
		exit(-1);
	}

	try
	{
		// install the signal handler
		install_signal_handler();
	} catch (std::exception & bad)
	{
		std::cerr << "Failed to install signal handlers!" << std::endl;
		exit(-1);
	}

	try
	{
		tissuestack::imaging::TissueStackLabelLookupStore::instance(); // for label lookups
		tissuestack::imaging::TissueStackColorMapStore::instance(); // the colormap store
	} catch (std::exception & bad)
	{
		std::cerr << "Failed to instantiate color map store: " << bad.what() << std::endl;
		std::cerr << "This will cause pre-tiling to fail IF you handed in a color map parameter to be applied!\n";
	}

	try
	{
		InitializeMagick(NULL);

		// extract comma separated dimensions
		std::vector<std::string> dims =
			tissuestack::utils::Misc::tokenizeString(
				tissuestack::utils::Misc::eliminateWhitespaceAndUnwantedEscapeCharacters(dimensions), ',');
		if (dims.empty()) // use x,y,z as defaults
			dims = {"xspace", "yspace", "zspace"};

		// extract comma separated zooms
		std::vector<std::string> zooms =
			tissuestack::utils::Misc::tokenizeString(
				tissuestack::utils::Misc::eliminateWhitespaceAndUnwantedEscapeCharacters(zoom_levels), ',');

		std::vector<unsigned short> numericZoomLevels;
		for (auto z : zooms)
			numericZoomLevels.push_back(static_cast<unsigned short>(atoi(z.c_str())));
		if (numericZoomLevels.empty()) // use 0,1,2,3,4,5,6,
			numericZoomLevels = {{0, 1, 2, 3, 4, 5, 6, 7}};

		// delegate to the offline executor
		OfflineExecutor->preTile(
			in_file,
			tile_dir,
			dims,
			numericZoomLevels,
			color_map);
	} catch (const std::exception & any)
	{
		std::cerr << "Failed to pre-tile: " << any.what() << std::endl;
	}

	cleanUp();

	exit(0);
}

