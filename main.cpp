#include <chrono>
#include <ctime>
#include <format>
#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>
#include <thread>
#include <typeinfo>
#include <vector>

using std::vector;

using namespace std::chrono_literals;
using namespace std::chrono;

class Scheduler {
  using job = void (*)();
  vector<std::thread> jobs;
  vector<
      std::pair<std::pair<system_clock::time_point, system_clock::time_point>,
                std::thread>>
      daily_jobs;
  vector<
      std::pair<std::pair<system_clock::time_point, system_clock::time_point>,
                std::thread>>
      weekly_jobs;
  vector<
      std::pair<std::pair<system_clock::time_point, system_clock::time_point>,
                std::thread>>
      monthly_jobs;
  system_clock::time_point termination_point{};
  duration<int, std::micro> min_period{};

  // TODO We initiate for each time period (day, week, month, year) our own
  // list. Every List contains pairs that contain a pair or string a thread.
  // When we schedule, we fill those lists through method calls.
  // For those special lists we implement the logik (in a method below) where we
  // check the current schedule time with the end time and schedule (depending
  // on the list) a new task with said period. We update the new scheduled time,
  // call join on the existing thread and put it back in the vector. Wen have to
  // update the old element of the vector. If an element is terminated we put
  // all nullptr or an empty pair in there (to not do costly cleaning) (Of
  // course if it makes sense we clean the vector). The update goes through a
  // while loop waiting by the smallest waiting time (Not sure if this is
  // enough) (Maybe it is necessary to safe the next due task a do a clean up
  // then).

  system_clock::time_point parse_time(std::string string_time_point,
                                      std::string forwarded_fmt) {
    std::tm tm = {};
    // This will allow for detection if Daylight Saving Time is
    tm.tm_isdst = -1;
    std::stringstream ss(string_time_point);
    ss >> std::get_time(&tm, const_cast<char *>(forwarded_fmt.c_str()));
    auto scheduled_time = system_clock::from_time_t(std::mktime(&tm));
    return scheduled_time;
  }

  void set_termination_point(system_clock::time_point point) {
    if (point > termination_point) {
      termination_point = point;
    }
  }

  // Test
  void set_period(microseconds period) {
    if (min_period == microseconds(0) || period < min_period) {
      min_period = period;
    }
  }

 public:
  /**
   * @brief Handles all until the call of this method added to jobs.
   *
   * This method must be called last to make sure
   * threads / scheduled jobs are handled properly.
   */
  void handle_schedule() {
    for (int i = 0; i < jobs.size(); i++) {
      if (jobs[i].joinable()) {
        jobs[i].join();
      }
    }

    jobs.clear();
  }

  /**
   * @brief Creates a thread with a timer and adds it to the jobs vector.
   *
   * This method must be called to create a new job.
   *
   * @param func A function pointer to the function that should executed.
   * @param delay The delay in std::chrono::microseconds of the call of the
   * function.
   */
  void schedule_in(job func, microseconds delay) {
    time_point<system_clock> now = system_clock::now();
    jobs.push_back(std::thread([func, delay, now]() {
      std::this_thread::sleep_until(now + delay);
      func();
    }));
  }

  /**
   * @brief Creates a thread with a timer and adds it to the jobs vector.
   *
   * This method must be called to create a new job.
   *
   * @param func A function pointer to the function that should executed.
   * @param delay The scheduled time for execution in string format
   * YYYY-MM-DDTHH:MM:SS
   */
  void schedule_at(job func, std::string string_time_point) {
    system_clock::time_point parsed_time_point =
        parse_time(string_time_point, "%FT%T");
    auto now = system_clock::now();
    duration<int, std::micro> delay =
        duration_cast<microseconds>(parsed_time_point - now);
    schedule_in(func, microseconds(delay.count()));
  }

  /**
   * @brief Creates a thread with a timer and adds it to the jobs vector.
   *
   * This method must be called to create a new job.
   *
   * @param func A function pointer to the function that should executed.
   * @param delay The scheduled time for execution in as
   * std::chrono::timepoint()
   */
  void schedule_at(job func, system_clock::time_point scheduled_time) {
    system_clock::time_point now = system_clock::now();
    duration<int, std::micro> delay =
        duration_cast<microseconds>(scheduled_time - now);
    schedule_in(func, microseconds(delay.count()));
  }

  void schedule_daily(job func, std::string string_time,
                      std::string string_end) {
    set_period(hours(24));

    // Get the current date as String to complete the ISO timestamp for today.
    auto current_calendar_time = std::time(nullptr);
    auto tm = *std::localtime(&current_calendar_time);
    std::ostringstream date_string_stream;
    date_string_stream << std::put_time(&tm, "%Y-%m-%d");
    std::string string_date = date_string_stream.str();

    // Initialize the first schedule time for today.
    system_clock::time_point scheduled_time =
        parse_time(string_date + "T" + string_time, "%FT%T");
    system_clock::time_point scheduled_end = parse_time(string_end, "%FT%T");

    set_termination_point(scheduled_end);

    auto now = system_clock::now();
    // TODO Do repeated events have to be scheduled from a separate thread ?
    // Will there be blocking behavior in some sense ? Does this mean I should
    // call that in a thread as well ? Maybe even a detached one ? It will
    // certainly not run longer than the program and its a background task in
    // some sense.
    while (now < scheduled_end) {
      duration<int, std::micro> delay =
          duration_cast<microseconds>(scheduled_time - now);
      schedule_in(func, microseconds(delay.count()));
      if (scheduled_time + hours(24) <= scheduled_end) {
        scheduled_time = scheduled_time + hours(24);
      }
    }
  }

  // TODO Weekly
  // TODO Monthly
  // TODO Error Handling
  // TODO Umbennen zu aScheduler.h
  // TODO Possibility to limit Parallelization
  // TODO identify const and constexpr
};

void test() {
  std::cout << "test started" << std::endl;
  std::this_thread::sleep_for(1000ms);
  std::cout << "test works" << std::endl;
}

void test2() {
  std::cout << "test2 started" << std::endl;
  std::this_thread::sleep_for(1000ms);
  std::cout << "test2 works" << std::endl;
}

void test3() {
  std::cout << "test3 started" << std::endl;
  std::this_thread::sleep_for(1000ms);
  std::cout << "test3 works" << std::endl;
}

int main() {
  // Get the current time
  const time_point<system_clock> now = system_clock::now();

  // Test the Scheduler class.
  Scheduler scheduler;

  scheduler.schedule_in(&test, seconds(5));
  scheduler.schedule_in(&test2, seconds(1));
  scheduler.schedule_in(&test3, seconds(10));

  scheduler.handle_schedule();

  return 0;
}
