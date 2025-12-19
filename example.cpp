#include <iostream>

#include "aScheduler.h"

using namespace std::chrono_literals;
using namespace std::chrono;


void funcA() {
  std::cout << "=== function A started executing important work ===" << std::endl;
  std::this_thread::sleep_for(1000ms);
  std::cout << "=== function A finished its important work ========" << std::endl;
}

void funcB() {
  std::cout << "=== function B started executing important work ===" << std::endl;
  std::this_thread::sleep_for(1000ms);
  std::cout << "=== function B finished its important work ========" << std::endl;
}

int main() {
  const time_point<system_clock> now = system_clock::now();
  AScheduler scheduler;
  // I recommend to test with your current time + 1 Minute for the first
  // timestamp as the starting point. As the second one is the termination point
  // you can set it to current time + 2 min
  // => test will run every second for 1 Minute in total.
  scheduler.schedule_secondly(funcA, "HH:MM", "YYYY-MM-DDTHH:MM", 1);
  // One example to use schedule_in would be to do some clean_up task after a
  // recurring task. However, using it independently is also possible.
  scheduler.schedule_in(funcB, seconds(10));

  scheduler.handle_schedule();

  return 0;
}
