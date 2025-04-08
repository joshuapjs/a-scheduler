#include <mutex>
#include <iostream>

#include "aScheduler.h"

using namespace std::chrono_literals;
using namespace std::chrono;

std::mutex m;

void test() {
  m.lock();
  std::cout << "test started" << std::endl;
  std::this_thread::sleep_for(1000ms);
  std::cout << "test works" << std::endl;
  m.unlock();
}

void test2() {
  m.lock();
  std::cout << "test2 started" << std::endl;
  std::this_thread::sleep_for(1000ms);
  std::cout << "test2 works" << std::endl;
  m.unlock();
}

void test3() {
  m.lock();
  std::cout << "test3 started" << std::endl;
  std::this_thread::sleep_for(1000ms);
  std::cout << "test3 works" << std::endl;
  m.unlock();
}

int main() {
  const time_point<system_clock> now = system_clock::now();
  AScheduler scheduler;

  scheduler.schedule_in(test, seconds(1));
  scheduler.schedule_in(test2, seconds(1));
  scheduler.schedule_in(test3, seconds(10));
  scheduler.schedule_hourly(test, "12:46", "2025-04-08T12:47", 1);

  scheduler.handle_schedule();

  return 0;
}
