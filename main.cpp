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

/**
 * @brief A POD to handle recurring jobs.
 */
struct RecurringJob {
  std::thread thread;
  system_clock::time_point termination_point;
  system_clock::time_point scheduled_date;
  void (*recurring_job)();
};

/**
 * @class AScheduler
 * @brief A class to schedule jobs.
 */
class AScheduler {
  /**
   * @brief A job is defined as a void function, given by its pointer.
   */
  using job = void (*)();
  /**
   * @brief Vector that contains threads holding their scheduled job.
   */
  vector<std::thread> jobs;
  /**
   * @brief Vector that contains threads holding their scheduled (recurring)
   * job. Each job repeats hourly and has a termination date.
   */
  vector<RecurringJob> hourly_jobs;
  /**
   * @brief Vector that contains threads holding their scheduled (recurring)
   * job. Each job repeats daily and has a termination date.
   */
  vector<RecurringJob> daily_jobs;
  /**
   * @brief Vector that contains threads holding their scheduled (recurring)
   * job. Each job repeats weekly and has a termination date.
   */
  vector<RecurringJob> weekly_jobs;
  /**
   * @brief Vector that contains threads holding their scheduled (recurring)
   * job. Each job repeats monthly and has a termination date.
   */
  vector<RecurringJob> monthly_jobs;
  /**
   * @brief Vector that contains threads holding their scheduled (recurring)
   * job. Each job repeats yearly and has a termination date.
   */
  vector<RecurringJob> yearly_jobs;
  /**
   * @brief Time point indicating the latest termination point of a recurring
   * job that was scheduled. This is necessary for terminating the program.
   */
  system_clock::time_point termination_point;
  /**
   * @brief Vector that contains threads holding their scheduled (recurring)
   * job. Each job repeats daily and has a termination date.
   */
  system_clock::time_point min_period;

  /**
   * @brief helper method to parse time leaning on ISO 8601 while the time zone
   * and offset is omitted.
   *
   * Daylight saving time (dst) is not specifically
   * omitted and might be determined by the system.
   *
   * @param string_time_point: Time give as string.
   * @param forwarded_fmt: The Format to interpret the string correctly, e.g.
   * %FT%T for "2025-04-06T23:14".
   *
   */
  system_clock::time_point parse_time(std::string string_time_point,
                                      std::string forwarded_fmt) {
    std::tm tm = {};
    // This will allow for detection if Daylight Saving Time is
    tm.tm_isdst = -1;
    std::stringstream ss(string_time_point);
    ss >> std::get_time(&tm, const_cast<char*>(forwarded_fmt.c_str()));
    auto scheduled_time = system_clock::from_time_t(std::mktime(&tm));
    return scheduled_time;
  }

  /**
   * @brief Handles the initial scheduling of a recurring job.
   *
   * @param func: A Job.
   * @param delay: A duration until the first run of the job.
   * @param termination_point: The time point by when rescheduling schould
   * finished. During all time points before this time point, beginning from the
   * first scheduled run a rescheduling is possible. For a controlled end I
   * would take a termination date near the last preferred execution.
   * @param period_id: Integer indicating if the recurrence should be hourly,
   * weekly, etc.
   */
  void schedule_recurring(job func, microseconds delay,
                          time_point<system_clock> termination_point,
                          int period_id) {
    time_point<system_clock> now = system_clock::now();
    vector<RecurringJob>* jobs_list{};
    set_period(now + delay);

    switch (period_id) {
      // hourly recurrency has id 0
      case 0:
        jobs_list = &hourly_jobs;
        break;

      // daily recurrance has id 1
      case 1:
        jobs_list = &daily_jobs;
        break;

      // weekly recurrance has id 2
      case 2:
        jobs_list = &weekly_jobs;
        break;

      // monthly recurrance has id 3
      case 3:
        jobs_list = &monthly_jobs;
        break;

      // yearly recurrance has id 4
      default:
        jobs_list = &yearly_jobs;
        break;
    }

    (*jobs_list)
        .push_back({.scheduled_date = now + delay,
                    .termination_point = termination_point,
                    .recurring_job = func,
                    .thread = std::thread([func, delay, now]() {
                      std::this_thread::sleep_until(now + delay);
                      func();
                    })});
  }

  /**
   * @brief updates a vector containing all jobs of a specific time-period
   * (e.g. daily, weekly).
   *
   * @param periodic_jobs_vector: A reference to a vector that contains all
   * jobs that are to be repeated within a certain period, e.g. daily or
   * weekly.
   * @param period: A std::chrono::<time period> element to indicate the waiting
   * time to the first scheduled run.
   */
  void handle_recurring_vector(vector<RecurringJob>& periodic_jobs_vector,
                               microseconds period) {
    time_point<system_clock> now;
    // While iterating over the vector containing the recurring jobs we can not
    // alter the vector we are iterating over. If job must to be rescheduled it
    // will be placed into the intermediate vector and moved to the cleaned,
    // correct vector after were are done.
    vector<RecurringJob> intermediate{};

    for (int i = 0; i < periodic_jobs_vector.size(); i++) {
      now = system_clock::now();
      time_point<system_clock> next_scheduled_time = now + period;

      // Check if the execution of a job already started.
      if (!(periodic_jobs_vector[i].scheduled_date < now)) {
        // If the execution started the thread is definetely joinable. Otherwise
        // something went very wrong.
        if (periodic_jobs_vector[i].thread.joinable()) {
          periodic_jobs_vector[i].thread.join();
        }
        // If another execution can be started before the termination_point it
        // will be scheduled.
        if (next_scheduled_time < periodic_jobs_vector[i].termination_point) {
          job unpacked_job = periodic_jobs_vector[i].recurring_job;
          intermediate.push_back(
              {.scheduled_date = next_scheduled_time,
               .termination_point = periodic_jobs_vector[i].termination_point,
               .recurring_job = periodic_jobs_vector[i].recurring_job,
               .thread =
                   std::thread([unpacked_job, next_scheduled_time, now]() {
                     std::this_thread::sleep_until(next_scheduled_time);
                     unpacked_job();
                   })});
        }
        // We mark the job as an empty RecurringJob to clean up efficiently.
        periodic_jobs_vector[i] = RecurringJob{};
      }
    }

    // Now that all elements are handled we can try to set our period as it
    // becomes microseconds(0) at the beginning of each while loop in
    // handle_schedule.
    set_period(now + period);

    if (!intermediate.empty()) {
      // Move all elements of the intermediate vector to the official vector.
      periodic_jobs_vector.insert(periodic_jobs_vector.end(),
                                  std::make_move_iterator(intermediate.begin()),
                                  std::make_move_iterator(intermediate.end()));
      // Elements that were moved are now in a specific moved state and will be
      // erased from their source.
      intermediate.erase(intermediate.begin(), intermediate.end());
    }

    // We remove all redundant elements from the vector containing the
    // RecurringJob Elements.
    time_point<system_clock> fallback_date{};
    periodic_jobs_vector.erase(
        std::remove_if(periodic_jobs_vector.begin(), periodic_jobs_vector.end(),
                       [fallback_date](const RecurringJob& x) {
                         return x.scheduled_date == fallback_date;
                       }),
        periodic_jobs_vector.end());
  }

  /**
   * @brief Initializes or updates the termination_point member variable.
   *
   * This method aims to set the member_variable termination_point as the
   * highest termination point of all recurring jobs.
   *
   * @param point: A std::chrono::system_clock::time_point element which could
   * be potentially the highest termination point of a recurring job.
   */
  void set_termination_point(const system_clock::time_point& point) {
    if (point > termination_point) {
      termination_point = point;
    }
  }

  /**
   * @brief Saves the scheduled execution time point of the nearest job.
   *
   * @param new_min_point: A time point of a scheduled job, potentially the
   * next smaller one.
   */
  void set_period(system_clock::time_point new_min_point) {
    system_clock::time_point now = system_clock::now();
    time_point<system_clock> fallback_time{};
    if (min_period == fallback_time ||
        (new_min_point < min_period) && (min_period > now)) {
      min_period = new_min_point;
      std::cout << new_min_point << std::endl;
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

    time_point<system_clock> now = system_clock::now();
    while (now < termination_point) {
      handle_recurring_vector(hourly_jobs, hours(1));
      handle_recurring_vector(daily_jobs, days(1));
      handle_recurring_vector(weekly_jobs, weeks(1));
      handle_recurring_vector(
          monthly_jobs, months(1));  
      handle_recurring_vector(yearly_jobs, years(1));
      std::this_thread::sleep_until(min_period);
      now = system_clock::now();
    }
  }

  /**
   * @brief Creates a thread with a timer and adds it to the jobs vector.
   *
   * This method must be called to create a new job.
   *
   * @param func: A function pointer to the function that should executed.
   * @param delay: The delay in std::chrono::microseconds of the call of the
   * function.
   */
  void schedule_in(job func, microseconds delay) {
    time_point<system_clock> now = system_clock::now();
    set_period(now + delay);
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
   * @param func: A function pointer to the function that should executed.
   * @param delay: The scheduled time for execution in string format
   * YYYY-MM-DDTHH:MM:SS
   */
  void schedule_at(job func, std::string string_time_point) {
    system_clock::time_point parsed_time_point =
        parse_time(string_time_point, "%FT%T");
    auto now = system_clock::now();
    duration<int, std::micro> delay =
        duration_cast<microseconds>(parsed_time_point - now);
    schedule_in(func, microseconds(delay.count()));
    set_period(now + delay);
  }

  /**
   * @brief Creates a thread with a timer and adds it to the jobs vector.
   *
   * This method must be called to create a new job.
   *
   * @param func: A function pointer to the function that should executed.
   * @param delay: The scheduled time for execution in as
   * std::chrono::timepoint()
   */
  void schedule_at(job func, system_clock::time_point scheduled_time) {
    system_clock::time_point now = system_clock::now();
    duration<int, std::micro> delay =
        duration_cast<microseconds>(scheduled_time - now);
    schedule_in(func, microseconds(delay.count()));
    set_period(now + delay);
  }

  /**
   * @brief schedules a task that repeats hourly.
   *
   * @param func: a Job.
   * @param string_time_point: Time give as string.
   * @param string_end: Time point given as a string indicating the last point
   * of time execution.
   */
  void schedule_hourly(job func, std::string string_time,
                       std::string string_end) {
    // Initialize the first schedule job for today.
    system_clock::time_point scheduled_time = parse_time(string_time, "%FT%T");
    system_clock::time_point termination_point =
        parse_time(string_end, "%FT%T");

    set_termination_point(termination_point);
    auto now = system_clock::now();

    duration<int, std::micro> delay =
        duration_cast<microseconds>(scheduled_time - now);

    set_period(now + delay);

    schedule_recurring(func, microseconds(delay.count()), termination_point, 0);
  }

  /**
   * @brief schedules a task that repeats daily.
   *
   * @param func: a Job.
   * @param string_time_point: Time give as string.
   * @param string_end: Time point given as a string indicating the last point
   * of time execution.
   */
  void schedule_daily(job func, std::string string_time,
                      std::string string_end) {
    // Get the current date as String to get the ISO timestamp for today.
    // I want that the user does not has to explicitly state if a task should
    // start today.
    auto current_calendar_time = std::time(nullptr);
    auto tm = *std::localtime(&current_calendar_time);
    std::ostringstream date_string_stream;
    date_string_stream << std::put_time(&tm, "%Y-%m-%d");
    std::string string_date = date_string_stream.str();

    // Initialize the first schedule job for today.
    system_clock::time_point scheduled_time =
        parse_time(string_date + "T" + string_time, "%FT%T");
    system_clock::time_point termination_point =
        parse_time(string_end, "%FT%T");

    set_termination_point(termination_point);
    auto now = system_clock::now();

    duration<int, std::micro> delay =
        duration_cast<microseconds>(scheduled_time - now);

    set_period(now + delay);

    schedule_recurring(func, microseconds(delay.count()), termination_point, 1);
  }

  /**
   * @brief schedules a task that repeats weekly.
   *
   * @param func: a Job.
   * @param string_time_point: Time give as string.
   * @param string_end: Time point given as a string indicating the last point
   * of time execution.
   */
  void schedule_weekly(job func, std::string string_time,
                       std::string string_end) {
    // Initialize the first schedule job.
    system_clock::time_point scheduled_time = parse_time(string_time, "%FT%T");
    system_clock::time_point termination_point =
        parse_time(string_end, "%FT%T");

    set_termination_point(termination_point);
    auto now = system_clock::now();

    duration<int, std::micro> delay =
        duration_cast<microseconds>(scheduled_time - now);

    set_period(now + delay);

    schedule_recurring(func, microseconds(delay.count()), termination_point, 2);
  }

  /**
   * @brief schedules a task that repeats monthly.
   *
   * @param func: a Job.
   * @param string_time_point: Time give as string.
   * @param string_end: Time point given as a string indicating the last point
   * of time execution.
   */
  void schedule_monthly(job func, std::string string_time,
                       std::string string_end) {
    // Initialize the first schedule job.
    system_clock::time_point scheduled_time = parse_time(string_time, "%FT%T");
    system_clock::time_point termination_point =
        parse_time(string_end, "%FT%T");

    set_termination_point(termination_point);
    auto now = system_clock::now();

    duration<int, std::micro> delay =
        duration_cast<microseconds>(scheduled_time - now);

    set_period(now + delay);

    schedule_recurring(func, microseconds(delay.count()), termination_point, 3);
  }

  /**
   * @brief schedules a task that repeats yearly.
   *
   * @param func: a Job.
   * @param string_time_point: Time give as string.
   * @param string_end: Time point given as a string indicating the last point
   * of time execution.
   */
  void schedule_yearly(job func, std::string string_time,
                       std::string string_end) {
    // Initialize the first schedule job.
    system_clock::time_point scheduled_time = parse_time(string_time, "%FT%T");
    system_clock::time_point termination_point =
        parse_time(string_end, "%FT%T");

    set_termination_point(termination_point);
    auto now = system_clock::now();

    duration<int, std::micro> delay =
        duration_cast<microseconds>(scheduled_time - now);

    set_period(now + delay);

    schedule_recurring(func, microseconds(delay.count()), termination_point, 4);
  }

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
  AScheduler scheduler;

  scheduler.schedule_in(test, seconds(5));
  scheduler.schedule_in(test2, seconds(1));
  scheduler.schedule_in(test3, seconds(10));
  scheduler.schedule_daily(test, "18:33", "2025-04-06T18:34");

  scheduler.handle_schedule();

  return 0;
}
