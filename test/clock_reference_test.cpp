
#define CATCH_CONFIG_MAIN

#include <thread>
#include <chrono>
#include "catch.hpp"
#include "src/clock_reference.h"
#include "src/dpl_error.h"

using namespace dpl;

TEST_CASE("Test clock reference ", "[ClockReference]" ) {

    dpl::IClockReference* clock = nullptr;
    int ret = dpl::GetClockReference(&clock);

    REQUIRE(ret == ERROR_SUCCESS);
    REQUIRE(clock != nullptr);

    SECTION("init pts", "[]") {

        REQUIRE(0 == clock->GetTime().count());
        REQUIRE(0 == clock->GetPts().count());

        clock->SeekTime(std::chrono::milliseconds(20));

        REQUIRE(20 == clock->GetTime().count());
        REQUIRE(20 == clock->GetPts().count());

        clock->SeekPts(std::chrono::milliseconds(20));

        REQUIRE(20 == clock->GetTime().count());
        REQUIRE(20 == clock->GetPts().count());
    }

    SECTION("pts seted", "[SetFirstPts]") {
        clock->SetFirstPts(std::chrono::milliseconds(20));

        REQUIRE(20 == clock->GetTime().count());
        REQUIRE(40 == clock->GetPts().count());

        clock->SeekTime(std::chrono::milliseconds(20));

        REQUIRE(20 == clock->GetTime().count());
        REQUIRE(40 == clock->GetPts().count());

        clock->SeekPts(std::chrono::milliseconds(20));

        REQUIRE(0 == clock->GetTime().count());
        REQUIRE(20 == clock->GetPts().count());
    }


    SECTION("Test start stop", "[StartClock]") {

        clock->StartClock();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        int time = clock->GetTime().count();
        REQUIRE(time >= 90);
        INFO(time);

        clock->StopClock();
    }

    SECTION("start with pause", "[TogglePause]") {

        clock->StartClock();


        clock->TogglePause();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        clock->TogglePause();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        int time = clock->GetTime().count();
        REQUIRE(time >= 90);
        INFO(time);

        clock->StopClock();
    }

}


