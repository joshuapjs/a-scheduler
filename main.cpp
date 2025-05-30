#include <iostream>
#include <mutex>

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
  // I recommend to test with your current time + 1 Minute for the first
  // timestamp as the starting point. As the second one is the termination point
  // you can set it to current time + 2 min
  // => test will run every second for 1 Minute in total.
  scheduler.schedule_secondly(test, "HH:MM", "YYYY-MM-DDTHH:MM", 1);
  // One example to use schedule_in would be to do some clean_up task after a
  // recurring task. However, using it independently is also possible.
  scheduler.schedule_in(test2, seconds(10));

  scheduler.handle_schedule();

  return 0;
}
