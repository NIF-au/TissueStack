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
	DestroyMagick();
}

std::unique_ptr<tissuestack::execution::TissueStackOfflineExecutor> OfflineExecutor(
		tissuestack::execution::TissueStackOfflineExecutor::instance());

void handle_signals(int sig) {
	switch (sig) {
		case SIGHUP:
		case SIGQUIT:
		case SIGTERM:
		case SIGINT:
			OfflineExecutor->stop();
			cleanUp();
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
	std::string in_file = "";
	std::string out_file = "";

	int c = 0;
	while (1)
	{
		static struct option long_options[] = {
			{"in",  required_argument, 0, 'i'},
			{"out", required_argument, 0, 'o'},
			{0, 0, 0, 0}
		};

		int option_index = 0;
		c = getopt_long (argc, argv, "i:o:", long_options, &option_index);
		if (c == -1)
			break;

		const std::string tmp =
			(optarg == NULL) ?
				"" :
				std::string(optarg, strlen(optarg));

		switch (c)
		{
			case 'i':
				in_file = tmp;
				break;

			case 'o':
				out_file = tmp;
				break;

			case '?':
				exit (0);   /* getopt_long already printed an error message. */
				break;

			default:
				std::cout << "Usage: " << argv[0] <<
					" -i IN_FILE (*.mnc,*.nii,*.nii.gz) -o OUT_FILE\n";
				exit(0);
		}
	}

	// check for mandatory params
	if (in_file.empty() || out_file.empty())
	{
		std::cerr << "Usage: " << argv[0] <<
			" -i IN_FILE (*.mnc,*.nii,*.nii.gz) -o OUT_FILE\n";
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
		InitializeMagick(NULL);

		//TODO: implement
		// delegate to the offline executor
		OfflineExecutor->convert(
			in_file,
			out_file);
	} catch (const std::exception & any)
	{
		std::cerr << "Failed to convert: " << any.what() << std::endl;
	}

	cleanUp();

	exit(0);
}
