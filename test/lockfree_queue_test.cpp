
#include <thread>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <utility>

#include "src/lockfree_queue.h"

using namespace dpl;

#define debug(msg) fprintf(stderr, "file:%s, func:%s, linc:%d : %s \n", __FILE__, __func__, __LINE__, msg)


void pop_thread_func(lockfree_queue<void*> &queue, int interval, bool &stoped)
{
    void* data;
    uint32_t pop_count = 0;
    int32_t log_interval = 5000 / interval;
    std::chrono::milliseconds dur(interval);

    printf("pop thread start\n");

    while(!stoped) {
        std::this_thread::sleep_for(dur);

        if(queue.pop(data)) {
            free(data);
        }

        if((pop_count++) % log_interval  == 0) {
            printf("pop thread on work\n");
        }
    }
}

void push_thread_func(lockfree_queue<void*> &queue, int interval, bool &stoped)
{
    void* data;
    uint32_t push_count = 0;
    int32_t log_interval = 5000 / interval;
    std::chrono::milliseconds dur(interval);

    printf("push thread start\n");

    while(!stoped) {
        std::this_thread::sleep_for(dur);

        if((data = malloc(200)) != nullptr) {
            if(!queue.push(data)) {
                free(data);
            }
        }

        if((push_count++) % log_interval == 0) {
            printf("push thread on work\n");
        }
    }
}

int main(void)
{
    lockfree_queue<void*> queue;
    bool stoped = false;
    int32_t push_interval = 1;
    int32_t pop_interval = 2;
    std::string input;

    std::thread pop_thread(pop_thread_func, std::ref(queue), pop_interval, std::ref(stoped));
    std::thread push_thread(push_thread_func, std::ref(queue), push_interval, std::ref(stoped));

    push_thread.detach();
    pop_thread.detach();

    while(true) {
        std::cout << "input stop, terminate the test" << std::endl;
        std::cin >> input;

        if(input == "stop") break;
    }

    return 0;
}
