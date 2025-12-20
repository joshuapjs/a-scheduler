#ifndef ASCHEDULER_H
#define ASCHEDULER_H

#include <chrono>
#include <ctime>
#include <format>
#include <iomanip>
#include <locale>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

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
  std::vector<std::thread> jobs;
  /**
   * @brief Vector that contains threads holding their scheduled (recurring)
   * job. Each job repeats hourly and has a termination date.
   */

  /**
   * @brief A POD to handle recurring jobs.
   */
  struct RecurringJob {
    std::thread thread;
    std::chrono::system_clock::time_point termination_point;
    std::chrono::system_clock::time_point scheduled_date;
    std::chrono::microseconds waiting_period;
    void (*recurring_job)();
  };

  /**
   * @brief Vector that contains threads holding their scheduled (recurring)
   * job. Each job repeats every n seconds and has a termination date.
   */
  std::vector<RecurringJob> secondly_jobs;
  /**
   * @brief Vector that contains threads holding their scheduled (recurring)
   * job. Each job repeats every n minutes and has a termination date.
   */
  std::vector<RecurringJob> minutely_jobs;
  /**
   * @brief Vector that contains threads holding their scheduled (recurring)
   * job. Each job repeats every n hours and has a termination date.
   */
  std::vector<RecurringJob> hourly_jobs;
  /**
   * @brief Vector that contains threads holding their scheduled (recurring)
   * job. Each job repeats every n days and has a termination date.
   */
  std::vector<RecurringJob> daily_jobs;
  /**
   * @brief Vector that contains threads holding their scheduled (recurring)
   * job. Each job repeats every n weeks and has a termination date.
   */
  std::vector<RecurringJob> weekly_jobs;
  /**
   * @brief Vector that contains threads holding their scheduled (recurring)
   * job. Each job repeats every n months and has a termination date.
   */
  std::vector<RecurringJob> monthly_jobs;
  /**
   * @brief Vector that contains threads holding their scheduled (recurring)
   * job. Each job repeats every n years and has a termination date.
   */
  std::vector<RecurringJob> yearly_jobs;
  /**
   * @brief Time point indicating the latest termination point of a recurring
   * job that was scheduled. This is necessary for terminating the program.
   */
  std::chrono::system_clock::time_point termination_point;
  /**
   * @brief Vector that contains threads holding their scheduled (recurring)
   * job. Each job repeats daily and has a termination date.
   */
  std::chrono::system_clock::time_point min_period;

  /**
   * @brief helper method to parse time leaning on ISO 8601 while the time zone
   * and offset is omitted.
   *
   * Daylight saving time (dst) is not specifically
   * omitted and might be determined by the system.
   *
   * @param string_time_point: Time given as string.
   * @param forwarded_fmt: The Format to interpret the string correctly, e.g.
   * %Y-%m-%dT%H:%M:%S for "2025-04-06T23:14".
   *
   */
  std::chrono::system_clock::time_point parse_time(
    std::string string_time_point, std::string forwarded_fmt) {
    std::tm tm = {};
    // This will allow for detection if Daylight Saving Time is
    tm.tm_isdst = -1;
    std::stringstream ss(string_time_point);
    ss >> std::get_time(&tm, const_cast<char*>(forwarded_fmt.c_str()));
    auto scheduled_time =
        std::chrono::system_clock::from_time_t(std::mktime(&tm));
    return scheduled_time;
  }

  /**
   * @brief Handles the initial scheduling of a recurring job.
   *
   * @param func: A Job.
   * @param string_time: Time given as string.
   * @param string_end: Time point given as a string indicating the last point
   * of time execution.
   * @param period_id: Integer indicating if the recurrence should be hourly,
   * weekly, etc.
   * @param waiting_period: The time we want to wait between two runs of a
   * recurring job.
   */
  void schedule_recurring(job func, std::string string_time,
                          std::string string_end, int period_id,
                          std::chrono::microseconds waiting_period) {
    std::chrono::system_clock::time_point scheduled_time;
    std::chrono::system_clock::time_point termination_point;
    // Check if the input string contains only HH:MM:SS
    // so no specific date using '-' with a designated time after 'T'
    if (string_time.find('T') == std::string::npos &&
        string_time.find('-') == std::string::npos) {
      auto current_calendar_time = std::time(nullptr);
      auto tm = *std::localtime(&current_calendar_time);
      std::ostringstream date_string_stream;
      date_string_stream << std::put_time(&tm, "%Y-%m-%d");
      std::string string_date = date_string_stream.str();

      // Initialize the first schedule job for today.
      scheduled_time = parse_time(string_date + "T" + string_time, "%Y-%m-%dT%H:%M:%S");
      termination_point = parse_time(string_end, "%Y-%m-%dT%H:%M:%S");
    } else {
      scheduled_time = parse_time(string_time, "%Y-%m-%dT%H:%M:%S");
      termination_point = parse_time(string_end, "%Y-%m-%dT%H:%M:%S");
    }

    std::chrono::time_point<std::chrono::system_clock> now =
        std::chrono::system_clock::now();

    set_termination_point(termination_point);
    set_period(scheduled_time);

    std::vector<RecurringJob>* jobs_list{};

    switch (period_id) {
      // secondly recurrency has id 0
      case 0:
        jobs_list = &secondly_jobs;
        break;

      // minutely recurrance has id 1
      case 1:
        jobs_list = &minutely_jobs;
        break;

      // hourly recurrance has id 2
      case 2:
        jobs_list = &hourly_jobs;
        break;

      // daily recurrance has id 3
      case 3:
        jobs_list = &daily_jobs;
        break;

      // weekly recurrance has id 4
      case 4:
        jobs_list = &weekly_jobs;
        break;

      // monthly recurrency has id 0
      case 5:
        jobs_list = &monthly_jobs;
        break;

      // yearly recurrency has id 0
      case 6:
        jobs_list = &yearly_jobs;
        break;
    }

    (*jobs_list).push_back({
      .thread = std::thread([func, scheduled_time]() {
        std::this_thread::sleep_until(scheduled_time);
        func();
      }),
      .termination_point = termination_point, .scheduled_date = scheduled_time,
      .waiting_period = waiting_period, .recurring_job = func
    });
  }

  /**
   * @brief updates a vector containing all jobs of a specific time-period
   * (e.g. daily, weekly).
   *
   * @param periodic_jobs_vector: A reference to a vector that contains all
   * jobs that are to be repeated within a certain period, e.g. daily or
   * weekly.
   */
  void handle_recurring_vector(
      std::vector<RecurringJob>& periodic_jobs_vector) {
    std::chrono::time_point<std::chrono::system_clock> now;
    // While iterating over the vector containing the recurring jobs we can not
    // alter the vector we are iterating over. If job must to be rescheduled it
    // will be placed into the intermediate vector and moved to the cleaned,
    // correct vector after were are done.
    std::vector<RecurringJob> intermediate{};

    for (int i = 0; i < periodic_jobs_vector.size(); i++) {
      now = std::chrono::system_clock::now();
      std::chrono::time_point<std::chrono::system_clock> next_scheduled_time =
          now + periodic_jobs_vector[i].waiting_period;

      if (periodic_jobs_vector[i].thread.joinable()) {
        periodic_jobs_vector[i].thread.join();
      }

      // Check if the execution of a job already started.

      if (periodic_jobs_vector[i].scheduled_date < now) {
        // If another execution can be started before the termination_point it
        // will be scheduled.
        if (next_scheduled_time < periodic_jobs_vector[i].termination_point) {
          job unpacked_job = periodic_jobs_vector[i].recurring_job;
          intermediate.push_back({
              .thread = std::thread([unpacked_job, next_scheduled_time, now]() {
                std::this_thread::sleep_until(next_scheduled_time);
                unpacked_job();
              }),
              .termination_point = periodic_jobs_vector[i].termination_point,
              .scheduled_date = next_scheduled_time,
              .waiting_period = periodic_jobs_vector[i].waiting_period,
              .recurring_job = periodic_jobs_vector[i].recurring_job,
          });
        }

        // We mark the job as an empty RecurringJob to clean up efficiently.
        periodic_jobs_vector[i] = RecurringJob{};
      }
    }

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
    std::chrono::time_point<std::chrono::system_clock> fallback_date{};
    periodic_jobs_vector.erase(
        std::remove_if(periodic_jobs_vector.begin(), periodic_jobs_vector.end(),
                       [fallback_date](const RecurringJob& x) {
                         return x.scheduled_date == fallback_date;
                       }),
        periodic_jobs_vector.end());

    // We have to find update the smallest period now
    for (int i = 0; i < periodic_jobs_vector.size(); i++) {
      now = std::chrono::system_clock::now();
      set_period(now + periodic_jobs_vector[i].waiting_period);
    }
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
  void set_termination_point(
      const std::chrono::system_clock::time_point& point) {
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
  void set_period(std::chrono::system_clock::time_point new_min_point) {
    std::chrono::system_clock::time_point now =
        std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock> fallback_time{};
    if (min_period == fallback_time ||
        (new_min_point < min_period) && (min_period > now)) {
      min_period = new_min_point;
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

    std::chrono::time_point<std::chrono::system_clock> now =
        std::chrono::system_clock::now();
    while (now < termination_point) {
      handle_recurring_vector(secondly_jobs);
      handle_recurring_vector(minutely_jobs);
      handle_recurring_vector(hourly_jobs);
      handle_recurring_vector(daily_jobs);
      handle_recurring_vector(weekly_jobs);
      handle_recurring_vector(monthly_jobs);
      handle_recurring_vector(yearly_jobs);
      std::this_thread::sleep_until(min_period);
      now = std::chrono::system_clock::now();
    }

    // It appears that sometimes threads are not joined properly.
    // Close to termination of the program I do not expect performance to be a critical issue.
    // I assume therefore that a clean up of all thread involved does not harm.
    for (int i = 0; i < secondly_jobs.size(); i++) {
      if (secondly_jobs[i].thread.joinable()) {
        secondly_jobs[i].thread.join();
      }
    }

    for (int i = 0; i < minutely_jobs.size(); i++) {
      if (minutely_jobs[i].thread.joinable()) {
        minutely_jobs[i].thread.join();
      }
    }

    for (int i = 0; i < hourly_jobs.size(); i++) {
      if (hourly_jobs[i].thread.joinable()) {
        hourly_jobs[i].thread.join();
      }
    }

    for (int i = 0; i < daily_jobs.size(); i++) {
      if (daily_jobs[i].thread.joinable()) {
        daily_jobs[i].thread.join();
      }
    }

    for (int i = 0; i < weekly_jobs.size(); i++) {
      if (weekly_jobs[i].thread.joinable()) {
        weekly_jobs[i].thread.join();
      }
    }

    for (int i = 0; i < monthly_jobs.size(); i++) {
      if (monthly_jobs[i].thread.joinable()) {
        monthly_jobs[i].thread.join();
      }
    }

    for (int i = 0; i < yearly_jobs.size(); i++) {
      if (yearly_jobs[i].thread.joinable()) {
        yearly_jobs[i].thread.join();
      }
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
  void schedule_in(job func, std::chrono::microseconds delay) {
    std::chrono::time_point<std::chrono::system_clock> now =
        std::chrono::system_clock::now();
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
   * @param string_time_point: The scheduled time for execution in string format
   * YYYY-MM-DDTHH:MM:SS
   */
  void schedule_at(job func, std::string string_time_point) {
    std::chrono::system_clock::time_point parsed_time_point =
        parse_time(string_time_point, "%FT%T");
    auto now = std::chrono::system_clock::now();
    std::chrono::duration<int, std::micro> delay =
        duration_cast<std::chrono::microseconds>(parsed_time_point - now);
    schedule_in(func, std::chrono::microseconds(delay.count()));
    set_period(now + delay);
  }

  /**
   * @brief Creates a thread with a timer and adds it to the jobs vector.
   *
   * This method must be called to create a new job.
   *
   * @param func: A function pointer to the function that should executed.
   * @param scheduled_time: The scheduled time for execution in as
   * std::chrono::timepoint()
   */
  void schedule_at(job func,
                   std::chrono::system_clock::time_point scheduled_time) {
    std::chrono::system_clock::time_point now =
        std::chrono::system_clock::now();
    std::chrono::duration<int, std::micro> delay =
        duration_cast<std::chrono::microseconds>(scheduled_time - now);
    schedule_in(func, std::chrono::microseconds(delay.count()));
    set_period(now + delay);
  }

  /**
   * @brief schedules a task that repeats every n seconds.
   *
   * @param func: a Job, given as a void function pointer.
   * @param string_time: Start time given as string. It is possible to use HH:MM for a start time today or YYYY-MM-DDTHH:MM:SS.
   * @param string_end: Time point given as a string indicating the last point
   * of time, an execution is allowed to start. The string must adhere to YYYY-MM-DDTHH:MM:SS.
   * @param period_multiple: The amount n of seconds to wait between two runs of a
   * job.
   */
  void schedule_secondly(job func, std::string string_time,
                       std::string string_end, int period_multiple) {
    schedule_recurring(func, string_time, string_end, 0,
                       std::chrono::seconds(period_multiple));
  }

  /**
   * @brief schedules a task that repeats every n minutes.
   *
   * @param func: a Job, given as a void function pointer.
   * @param string_time: Start time given as string. It is possible to use HH:MM for a start time today or YYYY-MM-DDTHH:MM:SS.
   * @param string_end: Time point given as a string indicating the last point
   * of time, an execution is allowed to start. The string must adhere to YYYY-MM-DDTHH:MM:SS.
   * @param period_multiple: The amount n of minutes to wait between two runs of a
   * job.
   */
  void schedule_minutely(job func, std::string string_time,
                       std::string string_end, int period_multiple) {
    schedule_recurring(func, string_time, string_end, 1,
                       std::chrono::minutes(period_multiple));
  }

  /**
   * @brief schedules a task that repeats every n hours.
   *
   * @param func: a Job, given as a void function pointer.
   * @param string_time: Start time given as string. It is possible to use HH:MM for a start time today or YYYY-MM-DDTHH:MM:SS.
   * @param string_end: Time point given as a string indicating the last point
   * of time, an execution is allowed to start. The string must adhere to YYYY-MM-DDTHH:MM:SS.
   * @param period_multiple: The amount n of hours to wait between two runs of a
   * job.
   */
  void schedule_hourly(job func, std::string string_time,
                       std::string string_end, int period_multiple) {
    schedule_recurring(func, string_time, string_end, 2,
                       std::chrono::hours(period_multiple));
  }

  /**
   * @brief schedules a task that repeats every n days.
   *
   * @param func: a Job, given as a void function pointer.
   * @param string_time: Start time given as string. It is possible to use HH:MM for a start time today or YYYY-MM-DDTHH:MM:SS.
   * @param string_end: Time point given as a string indicating the last point
   * of time, an execution is allowed to start. The string must adhere to YYYY-MM-DDTHH:MM:SS.
   * @param period_multiple: The amount n of days to wait between two runs of a
   * job.
   */
  void schedule_daily(job func, std::string string_time, std::string string_end,
                      int period_multiple) {
    schedule_recurring(func, string_time, string_end, 3,
                       std::chrono::days(period_multiple));
  }

  /**
   * @brief schedules a task that repeats every n weeks.
   *
   * @param func: a Job, given as a void function pointer.
   * @param string_time: Start time given as string. It is possible to use HH:MM for a start time today or YYYY-MM-DDTHH:MM:SS.
   * @param string_end: Time point given as a string indicating the last point
   * of time, an execution is allowed to start. The string must adhere to YYYY-MM-DDTHH:MM:SS.
   * @param period_multiple: The amount n of weeks to wait between two runs of a
   * job.
   */
  void schedule_weekly(job func, std::string string_time,
                       std::string string_end, int period_multiple) {
    schedule_recurring(func, string_time, string_end, 4,
                       std::chrono::weeks(period_multiple));
  }

  /**
   * @brief schedules a task that repeats every n months.
   *
   * @param func: a Job, given as a void function pointer.
   * @param string_time: Start time given as string. It is possible to use HH:MM for a start time today or YYYY-MM-DDTHH:MM:SS.
   * @param string_end: Time point given as a string indicating the last point
   * of time, an execution is allowed to start. The string must adhere to YYYY-MM-DDTHH:MM:SS.
   * @param period_multiple: The amount n of months to wait between two runs of a
   * job.
   */
  void schedule_monthly(job func, std::string string_time,
                        std::string string_end, int period_multiple) {
    schedule_recurring(func, string_time, string_end, 5,
                       std::chrono::months(period_multiple));
  }

  /**
   * @brief schedules a task that repeats every n years.
   *
   * @param func: a Job, given as a void function pointer.
   * @param string_time: Start time given as string. It is possible to use HH:MM for a start time today or YYYY-MM-DDTHH:MM:SS.
   * @param string_end: Time point given as a string indicating the last point
   * of time, an execution is allowed to start. The string must adhere to YYYY-MM-DDTHH:MM:SS.
   * @param period_multiple: The amount n of years to wait between two runs of a
   * job.
   */
  void schedule_yearly(job func, std::string string_time,
                       std::string string_end, int period_multiple) {
    schedule_recurring(func, string_time, string_end, 6,
                       std::chrono::years(period_multiple));
  }
};

#endif
