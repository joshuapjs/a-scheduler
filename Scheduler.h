#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <string>

using std::vector;

using namespace std::chrono_literals;

template<typename... Ts>
class Scheduler {
    using job = void (*)(Ts... );
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
    void schedule(job func, std::chrono::milliseconds delay, Ts... ts) {

        jobs.push_back( std::thread([func, delay, now, ts... ](){
                std::cout << "Job started" << std::endl;
                std::this_thread::sleep_until(now + delay);
                func(ts... );
                }
            )
        );
    }
};

