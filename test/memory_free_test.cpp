
#define CATCH_CONFIG_MAIN

#include <cstdio>
#include "catch.hpp"
#include "src/memory_free.h"

class TestFree
{
public:
	TestFree() 
	{
		printf("test free construct\n");
	}

	~TestFree() 
	{
		printf("test free distruct\n");
	}
};

TEST_CASE("Test the free func", "dpl_free")
{
    auto p = new TestFree;
    auto p2 = new TestFree[5];

    dpl_freep(p);
    dpl_freepa(p2);

    REQUIRE(p == NULL);
    REQUIRE(p2 == NULL);

    auto ap = new TestFree;
    auto ap2 = new TestFree[5];

    auto_freep(TestFree, ap);
    auto_freepa(TestFree, ap2);
}
