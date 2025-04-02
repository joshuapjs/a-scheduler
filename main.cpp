#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>
#include <format>
#include <locale>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <typeinfo>

using std::vector;

using namespace std::chrono_literals;

class Scheduler {
    using job = void (*)(int i);
    vector<std::thread> jobs;

    public:

    /**
     * @brief Handles all until the call of this method added to jobs.
     *
     * This method must be called last to make sure 
     * threads / scheduled jobs are handled properly.
     */
    void handle_schedule() {
        for (int i=0; i < jobs.size(); i++) {
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
     * @param delay The delay in std::chrono::microseconds of the call of the function.
     */
    void schedule(job func, std::chrono::microseconds delay, int i) {
        const std::chrono::time_point<std::chrono::system_clock> now = 
            std::chrono::system_clock::now();
        jobs.push_back( std::thread([func, delay, now, i](){
                std::cout << "Job started" << std::endl;
                std::this_thread::sleep_until(now + delay);
                func(i);
                }
            )
        );
    }

    /**
     * @brief Creates a thread with a timer and adds it to the jobs vector.
     *
     * This method must be called to create a new job.
     * 
     * @param func A function pointer to the function that should executed.
     * @param delay The scheduled time for execution in string format YYYY-MM-DDTHH:MM:SS
     */
    void schedule(job func, std::string string_time_point, int i) {
          std::tm tm = {};
          // This will allow for detection if Daylight Saving Time is
          tm.tm_isdst = -1;   
          std::stringstream ss(string_time_point);
          ss >> std::get_time(&tm, "%FT%T");
          auto scheduled_time = std::chrono::system_clock::from_time_t(std::mktime(&tm));
          auto now = std::chrono::system_clock::now();
          std::chrono::duration<int, std::micro> delay = 
              std::chrono::duration_cast<std::chrono::microseconds>(scheduled_time - now);
          schedule(func, std::chrono::microseconds(delay.count()), i);
    }

    /**
     * @brief Creates a thread with a timer and adds it to the jobs vector.
     *
     * This method must be called to create a new job.
     * 
     * @param func A function pointer to the function that should executed.
     * @param delay The scheduled time for execution in as std::chrono::timepoint()
     */
    void schedule(job func, std::chrono::system_clock::time_point scheduled_time, int i) {
          std::tm tm = {};
          // This will allow for detection if Daylight Saving Time is
          tm.tm_isdst = -1;   
          auto now = std::chrono::system_clock::now();
          std::chrono::duration<int, std::micro> delay = 
              std::chrono::duration_cast<std::chrono::microseconds>(scheduled_time - now);
          schedule(func, std::chrono::microseconds(delay.count()), i);
    }
};

void test(int i) {
    std::cout << "test started" << std::endl;
    std::this_thread::sleep_for(1000ms);
    std::cout << "test works" << std::endl;
}

void test2(int i) {
    std::cout << "test2 started" << std::endl;
    std::this_thread::sleep_for(1000ms);
    std::cout << "test2 works" << std::endl;
}

void test3(int i) {
    std::cout << "test3 started" << std::endl;
    std::this_thread::sleep_for(1000ms);
    std::cout << "test3 works" << std::endl;
}



int main() {
    
     // Get the current time
     const std::chrono::time_point<std::chrono::system_clock> now =
                 std::chrono::system_clock::now();

     // Test the Scheduler class.
     Scheduler scheduler;

     scheduler.schedule(&test, std::chrono::seconds(5), 1);
     scheduler.schedule(&test2, std::chrono::seconds(1), 1);
     scheduler.schedule(&test3, std::chrono::seconds(10), 1);

     scheduler.handle_schedule();

     return 0;
}

