#ifndef	__SYSTEM_H__
#define __SYSTEM_H__

#include <thread>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysinfo.h>

namespace tissuestack
{
  namespace utils
  {
    class System final
    {
      public:
        static const unsigned int getNumberOfCores();
        static const unsigned long long int getTotalRam();
        static const unsigned long long int getUsedRam();
        static const unsigned long long int getFreeRam();
    };
  }

}

#endif	/* __SYSTEM_H__ */
