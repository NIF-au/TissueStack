#ifndef	__SYSTEM_H__
#define __SYSTEM_H__

#include <thread>
#include <iostream>
#include <ctime>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <typeinfo>
#include <cxxabi.h>

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
      private:
        System();
        System & operator=(const System&) = delete;
        System(const System&) = delete;
    };

    class Misc final
    {
      public:
        static const std::string convertCharPointerToString(char * some_characters);
    	static const std::string demangleTypeIdName(const char * mangeledIdName);
      private:
    	Misc();
    	Misc & operator=(const Misc&) = delete;
    	Misc(const Misc&) = delete;
    };

    class Timer
      {
        public:
          // NANO SECONDS CONVERSION
          static const unsigned long long int NANO_SECONDS_PER_SEC = 1000000000;

          // TYPES OF TIMERS
          enum class Type
          {
              CLOCK_GET_TIME,
              CLOCK_TICKS
          };

          // FACTORY METHOD
          static Timer * const getInstance(Type type_of_timer);

          // VIRTUAL TIMER METHODS
          virtual void start() = 0;
          virtual const unsigned long long int stop() = 0;
          virtual ~Timer(){};
        protected:
          Timer(){};
      };

      class OrdinaryTimer : public Timer
      {
        friend class Timer;

        public:
          void start();
          const unsigned long long int stop();

        private:
          struct timespec _clock_start;
          struct timespec _clock_end;
          OrdinaryTimer() {};
      };

      class ClockTimer : public Timer
      {
        friend class Timer;

        public:
          void start();
          const unsigned long long int stop();

        private:
          clock_t _clock_start;
          clock_t _clock_end;
          ClockTimer() : _clock_start(0), _clock_end(0) {};
      };
    }
}

#endif	/* __SYSTEM_H__ */
