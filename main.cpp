#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <string>

using job = void (*)();
using std::vector;

using namespace std::chrono_literals;

class Scheduler {
    vector<int> jobs;

    public:

    void schedule(job func, std::chrono::seconds delay) {

        const std::chrono::time_point<std::chrono::system_clock> now = 
            std::chrono::system_clock::now();

        std::thread job_thread{ 
                [func, delay, now](){
                std::this_thread::sleep_until(now + delay);
                func();
                }
        };
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

     Scheduler scheduler;

     scheduler.schedule(&test, std::chrono::seconds(1));
     scheduler.schedule(&test2, std::chrono::seconds(3));
     scheduler.schedule(&test3, std::chrono::seconds(1));

     return 0;
}

