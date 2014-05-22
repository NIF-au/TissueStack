#include "execution.h"
#include <string>

tissuestack::execution::SharedLibraryFunctionCall::SharedLibraryFunctionCall(const std::string so_library_path) :
	_so_library_path(so_library_path),_so_handle(nullptr) {}

tissuestack::execution::SharedLibraryFunctionCall::~SharedLibraryFunctionCall()
{

}

void tissuestack::execution::SharedLibraryFunctionCall::init()
{
	dlerror();
	this->_so_handle = dlopen(this->_so_library_path.c_str(), RTLD_LAZY);
	char * error = dlerror();
	if (error != nullptr) std::cout << "Loading => " << error << std::endl;
}

/*
 *  Call ::process with a lambda like this one to call shared library functionality
 * ---------------------------------------------------------------------------------
 *
 * const std::function<void (const tissuestack::common::Request * request)> f =
 *		  [&so] (const tissuestack::common::Request * request)
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
		const std::function<void (const tissuestack::common::Request * request)> * functionality,
		const tissuestack::common::Request * request)
{
	if (functionality) ((*functionality)(request));
}

void tissuestack::execution::SharedLibraryFunctionCall::stop()
{
	dlerror();
	dlclose(this->_so_handle);
	char * error = dlerror();
	if (error != nullptr) std::cout << "Closing => " << error << std::endl;
}

void * const tissuestack::execution::SharedLibraryFunctionCall::callDlSym(std::string function_name)
{
	dlerror();
	void * ret = dlsym(this->_so_handle, function_name.c_str());
	char * error = dlerror();

	if (error) return nullptr;
	return ret;
}
