#include "execution.h"

int main(int argc, char * args[])
{
  tissuestack::execution::SharedLibraryFunctionCall so("/tmp/test.so");
  so.init();
  so.process();
  so.stop();
  return 0;
}
