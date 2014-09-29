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
#include "utils.h"


/* abstract factory method */
tissuestack::utils::Timer * const tissuestack::utils::Timer::getInstance(Type type_of_timer)
{
  if (type_of_timer == tissuestack::utils::Timer::Type::CLOCK_GET_TIME) return new tissuestack::utils::OrdinaryTimer();
  else if (type_of_timer == tissuestack::utils::Timer::Type::CLOCK_TICKS) return new tissuestack::utils::ClockTimer();

  return 0;
}

/* 'Ordinary' Timer Implementation */
void tissuestack::utils::OrdinaryTimer::start() {
  ::clock_gettime(CLOCK_MONOTONIC_RAW,&this->_clock_start);
}

const unsigned long long int tissuestack::utils::OrdinaryTimer::stop() {
  ::clock_gettime(CLOCK_MONOTONIC_RAW,&this->_clock_end);

  unsigned long long int nanosecs_start =
      static_cast<unsigned long long int>(this->_clock_start.tv_sec) * tissuestack::utils::Timer::NANO_SECONDS_PER_SEC
        + static_cast<unsigned long long int>(this->_clock_start.tv_nsec);
  unsigned long long int nanosecs_end =
      static_cast<unsigned long long int>(this->_clock_end.tv_sec) * tissuestack::utils::Timer::NANO_SECONDS_PER_SEC
        + static_cast<unsigned long long int>(this->_clock_end.tv_nsec);

  return (nanosecs_end - nanosecs_start);
}

/* Clock Timer Implementation */
void tissuestack::utils::ClockTimer::start() {
  this->_clock_start = ::clock();
}

const unsigned long long int tissuestack::utils::ClockTimer::stop() {
  this->_clock_end = ::clock();

  return static_cast<unsigned long long int>(((
      static_cast<double>(this->_clock_end - this->_clock_start) / CLOCKS_PER_SEC) * tissuestack::utils::Timer::NANO_SECONDS_PER_SEC) );
}
