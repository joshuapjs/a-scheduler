#include <iostream>
#include <catch2/catch_test_macros.hpp>
#include <atomic>

#include "aScheduler.h"

using namespace std::chrono_literals;
using namespace std::chrono;

std::atomic<int> testVariable = 0;

void test() {
  testVariable += 1;
}

void test2() {
  testVariable += 2;
}

std::string future_time_string(int seconds_to_add) {
  auto current_calendar_time = std::time(nullptr);
  current_calendar_time += seconds_to_add;
  auto tm = *std::localtime(&current_calendar_time);
  std::ostringstream date_string_stream;
  date_string_stream << std::put_time(&tm, "%Y-%m-%d");
  date_string_stream << "T" << std::put_time(&tm, "%H:%M:%S");
  std::string current_time = date_string_stream.str();
  return current_time;
}


TEST_CASE("Basic Functionality test") {

    SECTION("Test 1 - schedule_in") {
        REQUIRE(testVariable == 0);
        const time_point<system_clock> start = system_clock::now();
        AScheduler scheduleA;
        scheduleA.schedule_in(test, std::chrono::seconds(1));
        scheduleA.handle_schedule();
        const time_point<system_clock> stop = system_clock::now();

        auto timeDifference = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

        REQUIRE(testVariable == 1);
        REQUIRE(timeDifference >= std::chrono::milliseconds(1000));
    }

    SECTION("Test 2 - recurring - schedule_secondly") {
        AScheduler scheduleB;
        testVariable = 0;
        REQUIRE(testVariable == 0);
        auto start = future_time_string(1);
        auto end = future_time_string(6);
        scheduleB.schedule_secondly(test2, start, end, 1);
        scheduleB.handle_schedule();
        std::cout << "start was:  " << start << "\n";
        std::cout << "end was:    " << end << std::endl;
        REQUIRE(testVariable == 10);
    }
}
