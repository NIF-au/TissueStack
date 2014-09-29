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
#include "execution.h"

tissuestack::execution::WorkerThread::WorkerThread(
		std::function<void (tissuestack::execution::WorkerThread * assigned_worker)> wait_loop)
	: std::thread(wait_loop, this)
{
	this->_is_running = true;
}

bool tissuestack::execution::WorkerThread::isRunning() const
{
	return this->_is_running;
}

void tissuestack::execution::WorkerThread::stop()
{
	this->_is_running = false;
}
