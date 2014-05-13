#ifndef	__TIMER_H__
#define __TIMER_H__

#include <iostream>
#include <ctime>

namespace tissuestack
{
  namespace utils
  {
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
        ClockTimer() {};
    };
  }
}

#endif	/* __TIMER_H__ */
