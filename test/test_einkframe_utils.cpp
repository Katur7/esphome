#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <cmath>
#include <string>
#include <vector>

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

TEST_CASE("weather_icon — known states map to MDI codepoints") {
    CHECK(weather_icon("sunny") == "\U000F0599");
    CHECK(weather_icon("rainy") == "\U000F0597");
    CHECK(weather_icon("partlycloudy") == "\U000F0595");
    CHECK(weather_icon("snowy-rainy") == "\U000F067F");
    CHECK(weather_icon("clear-night") == "\U000F0594");
    CHECK(weather_icon("windy-variant") == "\U000F059E");
}

TEST_CASE("weather_icon — unknown state returns empty string") {
    CHECK(weather_icon("not-a-real-state") == "");
    CHECK(weather_icon("") == "");
}

TEST_CASE("ui_icon — known keys map to MDI codepoints") {
    CHECK(ui_icon("mdi-cloud-percent") == "\U000F1A35");
    CHECK(ui_icon("mdi-umbrella") == "\U000F054A");
    CHECK(ui_icon("mdi-sun-wireless-outline") == "\U000F17FF");
    CHECK(ui_icon("mdi-battery") == "\U000F0079");
}

TEST_CASE("ui_icon — unknown key returns empty string") {
    CHECK(ui_icon("mdi-nonexistent") == "");
    CHECK(ui_icon("") == "");
}

// Fake measurer: width = 1 per character. Makes test widths trivially predictable.
static const TextMeasurer char_count_measurer = [](const std::string& s) {
    return static_cast<int>(s.length());
};

TEST_CASE("wrap_text_pure — short text that fits returns single line") {
    auto lines = wrap_text_pure("hello", 10, char_count_measurer);
    REQUIRE(lines.size() == 1);
    CHECK(lines[0] == "hello");
}

TEST_CASE("wrap_text_pure — multi-word text that fits returns single line") {
    auto lines = wrap_text_pure("hello world", 20, char_count_measurer);
    REQUIRE(lines.size() == 1);
    CHECK(lines[0] == "hello world");
}

TEST_CASE("wrap_text_pure — wraps at word boundary when exceeding width") {
    // "one two three" = 13 chars. max_width=7. "one two"=7 fits, adding " three" does not.
    auto lines = wrap_text_pure("one two three", 7, char_count_measurer);
    REQUIRE(lines.size() == 2);
    CHECK(lines[0] == "one two");
    CHECK(lines[1] == "three");
}

TEST_CASE("wrap_text_pure — multiple wraps") {
    // max_width=5. "one"=3 ok. "one two"=7 too wide → flush "one", start "two".
    // "two"=3 ok. "two three"=9 too wide → flush "two", start "three".
    // "three"=5 ok. "three four"=10 too wide → flush "three", start "four".
    auto lines = wrap_text_pure("one two three four", 5, char_count_measurer);
    REQUIRE(lines.size() == 4);
    CHECK(lines[0] == "one");
    CHECK(lines[1] == "two");
    CHECK(lines[2] == "three");
    CHECK(lines[3] == "four");
}

TEST_CASE("wrap_text_pure — single word longer than max_width emitted on its own line") {
    // "supercalifragilistic"=20, max_width=5. Word never fits but is emitted alone.
    auto lines = wrap_text_pure("supercalifragilistic", 5, char_count_measurer);
    REQUIRE(lines.size() == 1);
    CHECK(lines[0] == "supercalifragilistic");
}

TEST_CASE("wrap_text_pure — long word mid-sequence flushes and stands alone") {
    auto lines = wrap_text_pure("hi supercalifragilistic bye", 5, char_count_measurer);
    REQUIRE(lines.size() == 3);
    CHECK(lines[0] == "hi");
    CHECK(lines[1] == "supercalifragilistic");
    CHECK(lines[2] == "bye");
}

TEST_CASE("wrap_text_pure — empty input returns empty vector") {
    auto lines = wrap_text_pure("", 10, char_count_measurer);
    CHECK(lines.empty());
}

TEST_CASE("wrap_text_pure — leading/trailing whitespace collapsed (istringstream behavior)") {
    auto lines = wrap_text_pure("  hello  world  ", 20, char_count_measurer);
    // Whole string length = 16 > 20? No, 16 <= 20, so early return with original text preserved.
    REQUIRE(lines.size() == 1);
    CHECK(lines[0] == "  hello  world  ");
}

TEST_CASE("wrap_text_pure — whitespace collapsed when wrapping kicks in") {
    // Force loop path: width 5 < total length. istringstream drops whitespace.
    auto lines = wrap_text_pure("  hello  world  ", 5, char_count_measurer);
    // "hello"=5 ok. "hello world"=11 too wide → flush "hello", start "world".
    REQUIRE(lines.size() == 2);
    CHECK(lines[0] == "hello");
    CHECK(lines[1] == "world");
}
