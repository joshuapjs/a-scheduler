#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <string>

using job = void (*)();
using std::vector;

using namespace std::chrono_literals;

class Scheduler {
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
     * @param func A function pointer to a function that should executed.
     * @param delay The delay of the call of the function.
     */
    void schedule(job func, std::chrono::seconds delay) {
        const std::chrono::time_point<std::chrono::system_clock> now = 
            std::chrono::system_clock::now();
        jobs.emplace_back( std::thread([func, delay, now](){
                std::cout << "Job started" << std::endl;
                std::this_thread::sleep_until(now + delay);
                func();
                }
            )
        );
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
     const std::chrono::time_point<std::chrono::system_clock> now =
                 std::chrono::system_clock::now();

     // Test the Scheduler class.
     Scheduler scheduler;

     scheduler.schedule(&test, std::chrono::seconds(5));
     scheduler.schedule(&test2, std::chrono::seconds(1));
     scheduler.schedule(&test3, std::chrono::seconds(10));

     scheduler.handle_schedule();

     return 0;
}

