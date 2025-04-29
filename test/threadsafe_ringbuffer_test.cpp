
#define CATCH_CONFIG_MAIN

#include <string.h>

#include "src/threadsafe_ringbuffer.h" 
#include "catch.hpp"

using namespace dpl;

const char article[] = "Avery Brundage (1887–1975) was the fifth president of the International"
"Olympic Committee (IOC), the only American to hold that office."
"In 1912, he competed in the Summer Olympics, contesting the pentathlon and decathlon; "
"both events were won by Jim Thorpe. Brundage became a sports administrator, "
"rising rapidly through the ranks in U.S. sports groups.";

constexpr int ring_buffer_size = 2000;


TEST_CASE( "test the ringbuffer logic", "[single-file]" ) {
    threadsafe_ringbuffer<char> ring_buffer(ring_buffer_size);
    char* buffer = new char[sizeof(article)];

    INFO("the article length is " << sizeof(article));
    INFO("the ring buffer length is " << ring_buffer_size);
    
    SECTION("repeat push and pop, test if the logic is right") {
        for(int i = 0; i < 100; i++) {
            ring_buffer.push(article, sizeof(article));
            ring_buffer.pop(buffer, sizeof(article));

            INFO("the loop index: " << i);
            INFO("pop buffer: " << buffer);
            INFO("readable: " << ring_buffer.readable());
            INFO("writeable: " << ring_buffer.writeable());

            REQUIRE(strcmp(article, (const char*)buffer) == 0);
            
        }
    }
}