# a-scheduler

A multithreaded job scheduler written in cpp. You can use this single-file library in your project if you want e.g. a function to run at a specific time or even at a recurring basis.

 If you can't make it to the other sections of this `README`, please do me a favor and take a look at the examples, the License and adhere to the time format `YYYY-MM-DDTHH:MM:SS`. For that start date, if today's date should be inferred, just give a timestamp of format `HH:MM:SS`.

# Features

A job is defined as a `void` function that does not receive any arguments. You can enter a specific amount of time as delay. I recommend to use a helper type like e.g. `std::chrono::minutes(1)` for this. If it must be more specific you can enter a time point, sticking to the timeformat as mentioned. Beyond that you can set recurring tasks by specifying an amount of hours, days, weeks, months or years.

# Prerequisites

The complete project uses only the standard libraries of C++20. I recommend using macOS or Linux.

# Usage

Here are some examples. You can also build `main.cpp` and play around with it. Let `test` be a test function `void test()`:

```cpp
// function, delay
scheduler.schedule_in(test, std::chrono::seconds(10));

// function, start time
scheduler.schedule_at(test, "2025-04-08T15:01:01");

// specific time points are supported.
std::system_clock::time_point start = std::system_clock::now() + std::chrono::seconds(42);
// function, std::time_point
scheduler.schedule_at(test, start);

// function, start time, termination time, amount of hours
scheduler.schedule_hourly(test, "15:01", "2025-04-08T15:01:01", 1);
```

I recommend using `std::mutex` to lock the threads during the time they access shared resources. However, I wanted to leave this up to you. Make sure that the start date and time of jobs is not in the past. You are theoretically allowed to run an infinate amount of threads simultaneously so becareful. 

For recurring tasks there will be a check to see if, given the respective waiting period, another run can be started before the termination time point. It is therefore possible that your program runs longer than the termination period. However, if another run does not fit, the program can also run longer than necessary, so set the termination point close to the desired last scheduled run.

# License 

This project is licensed by the GNU Licence. Please visit [LICENSE](docs/LICENSE.md) for further information.