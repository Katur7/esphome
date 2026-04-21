#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <cmath>

#include "einkframe_utils.h"

TEST_CASE("minutes_to_next_slot — normal gap between slots") {
    CHECK(minutes_to_next_slot(8, 30) == 150);   // 08:30 → 11:00
    CHECK(minutes_to_next_slot(12, 0) == 180);   // 12:00 → 15:00
    CHECK(minutes_to_next_slot(16, 15) == 165);  // 16:15 → 19:00
}

TEST_CASE("minutes_to_next_slot — exactly on a slot skips to the next") {
    CHECK(minutes_to_next_slot(7, 0) == 240);    // 07:00 → 11:00
    CHECK(minutes_to_next_slot(11, 0) == 240);   // 11:00 → 15:00
    CHECK(minutes_to_next_slot(15, 0) == 240);   // 15:00 → 19:00
    CHECK(minutes_to_next_slot(19, 0) == 720);   // 19:00 → 07:00 next day
}

TEST_CASE("minutes_to_next_slot — one minute before a slot") {
    CHECK(minutes_to_next_slot(6, 59) == 1);
    CHECK(minutes_to_next_slot(10, 59) == 1);
    CHECK(minutes_to_next_slot(18, 59) == 1);
}

TEST_CASE("minutes_to_next_slot — midnight wraps to 07:00") {
    CHECK(minutes_to_next_slot(0, 0) == 420);
}

TEST_CASE("minutes_to_next_slot — after last slot wraps to 07:00 next day") {
    CHECK(minutes_to_next_slot(19, 1) == 719);   // 19:01 → 07:00 (+1 day)
    CHECK(minutes_to_next_slot(23, 30) == 450);  // 23:30 → 07:00
}

TEST_CASE("voltage_to_battery_percent — exact table values") {
    CHECK(voltage_to_battery_percent(4.13f) == 100.0f);
    CHECK(voltage_to_battery_percent(3.49f) == 5.0f);
    CHECK(voltage_to_battery_percent(3.90f) == 55.0f);
}

TEST_CASE("voltage_to_battery_percent — between tiers falls to lower bucket") {
    CHECK(voltage_to_battery_percent(4.12f) == 95.0f);  // below 4.13 tier
    CHECK(voltage_to_battery_percent(4.01f) == 75.0f);  // below 4.02 tier
}

TEST_CASE("voltage_to_battery_percent — clamps high voltage to 100") {
    CHECK(voltage_to_battery_percent(4.20f) == 100.0f);
    CHECK(voltage_to_battery_percent(5.00f) == 100.0f);
}

TEST_CASE("voltage_to_battery_percent — clamps low voltage to 0") {
    CHECK(voltage_to_battery_percent(2.50f) == 0.0f);
    CHECK(voltage_to_battery_percent(3.00f) == 0.0f);
}

TEST_CASE("voltage_to_battery_percent — invalid voltage returns NaN") {
    CHECK(std::isnan(voltage_to_battery_percent(0.0f)));
    CHECK(std::isnan(voltage_to_battery_percent(-1.0f)));
}

TEST_CASE("voltage_to_battery_percent — boundary values") {
    CHECK(voltage_to_battery_percent(3.09f) == 0.0f);   // lower clamp
    CHECK(voltage_to_battery_percent(4.14f) == 100.0f); // upper clamp
}
