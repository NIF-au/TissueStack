#include "execution.h"
#include "networking.h"

int main(int argc, char * args[])
{
  tissuestack::execution::SharedLibraryFunctionCall so("/tmp/test.so");
  so.init();

  const std::function<void (const tissuestack::common::Request * request, tissuestack::common::ProcessingStrategy * _this)> f =
 		  [&so] (const tissuestack::common::Request * request, tissuestack::common::ProcessingStrategy * _this)
 		  {
			const void * dlSymReturn = so.callDlSym(std::string("lala"));
			// check return
			if (dlSymReturn == nullptr) return;

			// cast to desired type
			int (*test)(const char *) = (int (*) (const char *)) dlSymReturn;


			std::string t("Hello World");
			std::cout << "Before Call: " << t << std::endl;
			test(t.c_str());
			std::cout << "After  Call: " << t << std::endl;
 		  };

  so.process(&f, nullptr);
  so.stop();

  return 1;
}
