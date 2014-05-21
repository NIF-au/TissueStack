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

void tissuestack::execution::SharedLibraryFunctionCall::process()
{
	dlerror();
	int (*test)(const char *) = (int (*) (const char *)) dlsym(this->_so_handle, "lala");
	char * error = dlerror();
	if (error != nullptr) std::cout << "Sym => " << error << std::endl;

	std::cout << "Before Call ..." << std::endl;
	std::string t("Hello World");
	int ret = test(t.c_str());

	std::cout << "After Call ..." << ret << std::endl;

}

void tissuestack::execution::SharedLibraryFunctionCall::stop()
{
	dlerror();
	dlclose(this->_so_handle);
	char * error = dlerror();
	if (error != nullptr) std::cout << "Closing => " << error << std::endl;
}
