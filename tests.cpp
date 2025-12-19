#include <iostream>
#include <catch2/catch_test_macros.hpp>
#include <atomic>

#include "aScheduler.h"

using namespace std::chrono_literals;
using namespace std::chrono;

int testVariable = 0;

void test() {
  testVariable += 1;
}

void test2() {
  testVariable += 2;
}

std::string future_time_string(int seconds_to_add)
{
    // 1. Get current time_point (system clock, UTC)
    auto now = std::chrono::system_clock::now();

    // 2. Add the offset
    auto future = now + std::chrono::seconds(seconds_to_add);

    // 3. Convert to std::time_t (seconds since epoch) for formatting
    std::time_t future_time_t = std::chrono::system_clock::to_time_t(future);

    // 4. Build a string in ISO‑8601 format (UTC, trailing 'Z')
    std::ostringstream oss;
    // gmtime converts to UTC (instead of localtime)
    std::tm utc_tm = *std::gmtime(&future_time_t);
    oss << std::put_time(&utc_tm, "%Y-%m-%dT%H:%M:%SZ");

    return oss.str();
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
        const time_point<system_clock> start = system_clock::now();
        AScheduler scheduleB;
        testVariable = 0;
        REQUIRE(testVariable == 0);
        scheduleB.schedule_secondly(test2, future_time_string(1), future_time_string(5), 1);
        scheduleB.handle_schedule();
        REQUIRE(testVariable > 0);
    }
}
