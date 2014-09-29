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

tissuestack::execution::SharedLibraryFunctionCall::SharedLibraryFunctionCall(const std::string so_library_path) :
	_so_library_path(so_library_path),_so_handle(nullptr) {}

tissuestack::execution::SharedLibraryFunctionCall::~SharedLibraryFunctionCall()
{
	// no need to do anything
}

void tissuestack::execution::SharedLibraryFunctionCall::init()
{
	dlerror(); // reset error and get a handle on the shared library

	this->_so_handle = dlopen(this->_so_library_path.c_str(), RTLD_LAZY);
	char * error = dlerror();

	if (error) {
		tissuestack::logging::TissueStackLogger::instance()->error("Error Loading Shared Library: %s\n", error);
		return;
	}
	// let's mark us as running, if we have come that far and haven't been undloaded yet
	if (!this->isStopFlagRaised())
		this->setRunningFlag(true);
}

/*
 *  Call ::process with a lambda like this one to call shared library functionality
 * ---------------------------------------------------------------------------------
 *
  * const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> f =
 *		  [&so] (const tissuestack::common::ProcessingStrategy * _this)
 *		  {
 *			const void * dlSymReturn = so.callDlSym(std::string("lala"));
 *			// check return
 *			if (dlSymReturn == nullptr) return;
 *
 *			// cast to desired type
 *			int (*test)(const char *) = (int (*) (const char *)) dlSymReturn;
 *
 *			// call shared library function
 *			std::string t("Hello World");
 *			test(t.c_str());
 *		  };
 *
 */
void tissuestack::execution::SharedLibraryFunctionCall::process(
		const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * functionality)
{
	// delegate only if we haven't received a null pointer for a closure
	// AND if we have been loaded already and not been unloaded meanwhile
	if (this->isRunning() && !this->isStopFlagRaised() && functionality)
		((*functionality)(this));
}

void tissuestack::execution::SharedLibraryFunctionCall::stop()
{
	dlerror(); // reset error
	// close/unload shared library
	dlclose(this->_so_handle);

	//set flags
	this->raiseStopFlag();
	this->setRunningFlag(false);

	// check for error
	char * error = dlerror();
	if (error != nullptr)
		tissuestack::logging::TissueStackLogger::instance()->error("Error Closing Shared Library: %s\n", error);
}

void * const tissuestack::execution::SharedLibraryFunctionCall::callDlSym(std::string function_name)
{
	// preliminary check, just in case we got stopped/unloaded meanwhile
	if (!this->isRunning() || this->isStopFlagRaised())
		return nullptr;

	dlerror(); // reset error
	// make call to function
	void * ret = dlsym(this->_so_handle, function_name.c_str());
	char * error = dlerror();

	// return null pointer on error, a function pointer otherwise
	if (error)
	{
		tissuestack::logging::TissueStackLogger::instance()->error("Error Getting Shared Library Function Handle: %s\n", error);
		return nullptr;
	}
	return ret;
}
