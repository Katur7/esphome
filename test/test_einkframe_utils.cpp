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

TEST_CASE("wrap_text_pure — hard_break splits oversized word with hyphens") {
    // 10-char word, max_width=5. Loop tries cur+ch+"-" each step; flushes
    // "abcd-" when adding "e" would push trial to width 6.
    auto lines = wrap_text_pure("abcdefghij", 5, char_count_measurer, /*hard_break=*/true);
    REQUIRE(lines.size() == 3);
    CHECK(lines[0] == "abcd-");
    CHECK(lines[1] == "efgh-");
    CHECK(lines[2] == "ij");  // tail line has no trailing hyphen
}

TEST_CASE("wrap_text_pure — hard_break never splits a UTF-8 multibyte char") {
    // "ð" is 2 bytes (0xC3 0xB0). With byte-counting measurer at max_width=3,
    // each line must be valid UTF-8 — no orphan 0xC3 or 0xB0.
    auto lines = wrap_text_pure("aðbðc", 3, char_count_measurer, /*hard_break=*/true);
    for (const auto& line : lines) {
        for (size_t i = 0; i < line.size(); i++) {
            unsigned char b = static_cast<unsigned char>(line[i]);
            // Continuation bytes (10xxxxxx) must be preceded by a lead byte.
            if ((b & 0xC0) == 0x80) {
                REQUIRE(i > 0);
                unsigned char prev = static_cast<unsigned char>(line[i - 1]);
                CHECK((prev & 0xC0) != 0x00);  // prev is non-ASCII lead or continuation
                CHECK(prev >= 0x80);
            }
        }
    }
    // Concatenating dropped hyphens reconstructs the input.
    std::string rejoined;
    for (auto& line : lines) {
        rejoined += (line.size() > 0 && line.back() == '-') ? line.substr(0, line.size() - 1) : line;
    }
    CHECK(rejoined == "aðbðc");
}

TEST_CASE("wrap_text_pure — hard_break with width too narrow for char+hyphen emits raw chars") {
    // max_width=1. Even "a-"=2 exceeds. Each char must be emitted alone, no hyphen.
    auto lines = wrap_text_pure("abcd", 1, char_count_measurer, /*hard_break=*/true);
    REQUIRE(lines.size() == 4);
    CHECK(lines[0] == "a");
    CHECK(lines[1] == "b");
    CHECK(lines[2] == "c");
    CHECK(lines[3] == "d");
}

TEST_CASE("split_translations — comma-separated") {
    auto items = split_translations("a, b, c");
    REQUIRE(items.size() == 3);
    CHECK(items[0] == "a");
    CHECK(items[1] == "b");
    CHECK(items[2] == "c");
}

TEST_CASE("split_translations — middle-dot separator") {
    auto items = split_translations("a · b · c");
    REQUIRE(items.size() == 3);
    CHECK(items[0] == "a");
    CHECK(items[1] == "b");
    CHECK(items[2] == "c");
}

TEST_CASE("split_translations — mixed separators") {
    auto items = split_translations("a, b · c, d");
    REQUIRE(items.size() == 4);
    CHECK(items[0] == "a");
    CHECK(items[3] == "d");
}

TEST_CASE("split_translations — no separators") {
    auto items = split_translations("single phrase");
    REQUIRE(items.size() == 1);
    CHECK(items[0] == "single phrase");
}

TEST_CASE("split_translations — empty and whitespace-only") {
    CHECK(split_translations("").empty());
    CHECK(split_translations("   ").empty());
    CHECK(split_translations(", ,, ·").empty());
}

TEST_CASE("split_translations — preserves multi-word phrases") {
    auto items = split_translations("arbeta på <uppgiften>, vinna i längden");
    REQUIRE(items.size() == 2);
    CHECK(items[0] == "arbeta på <uppgiften>");
    CHECK(items[1] == "vinna i längden");
}

TEST_CASE("split_translations — adjacent separators yield no empties") {
    auto items = split_translations("a,, b");
    REQUIRE(items.size() == 2);
    CHECK(items[0] == "a");
    CHECK(items[1] == "b");
}

TEST_CASE("cap_translations — under cap returns all, no overflow") {
    auto result = cap_translations({"a", "b", "c"}, 15);
    CHECK(result.visible.size() == 3);
    CHECK(result.overflow == 0);
}

TEST_CASE("cap_translations — exactly at cap returns all, no overflow") {
    std::vector<std::string> items(15, "x");
    auto result = cap_translations(items, 15);
    CHECK(result.visible.size() == 15);
    CHECK(result.overflow == 0);
}

TEST_CASE("cap_translations — over cap truncates and counts overflow") {
    std::vector<std::string> items(28, "x");
    auto result = cap_translations(items, 15);
    CHECK(result.visible.size() == 15);
    CHECK(result.overflow == 13);
}

TEST_CASE("cap_translations — empty input") {
    auto result = cap_translations({}, 15);
    CHECK(result.visible.empty());
    CHECK(result.overflow == 0);
}

TEST_CASE("cap_translations — zero cap drops everything to overflow") {
    auto result = cap_translations({"a", "b"}, 0);
    CHECK(result.visible.empty());
    CHECK(result.overflow == 2);
}

// Sized measurer: width = (size_index + 1) per character.
// Ladder indices: 0 = smallest (1 px/char), N-1 = largest.
static const SizedTextMeasurer linear_sized_measurer = [](int idx, const std::string& s) {
    return static_cast<int>(s.length()) * (idx + 1);
};

TEST_CASE("fit_headword_size_index — short text fits at largest size") {
    // "abc" at idx 5 → 18, at idx 4 → 15. Width budget 20 → idx 5 fits.
    CHECK(fit_headword_size_index("abc", 6, 20, linear_sized_measurer) == 5);
}

TEST_CASE("fit_headword_size_index — long text drops to a middle size") {
    // "abcdefghij" (10 chars). Width 30. idx 2 → 30 fits, idx 3 → 40 no.
    CHECK(fit_headword_size_index("abcdefghij", 6, 30, linear_sized_measurer) == 2);
}

TEST_CASE("fit_headword_size_index — overflowing text returns smallest index") {
    // 100-char string, width 10 → even idx 0 (1 px/char × 100 = 100) overflows.
    std::string huge(100, 'x');
    CHECK(fit_headword_size_index(huge, 6, 10, linear_sized_measurer) == 0);
}

TEST_CASE("fit_headword_size_index — empty ladder returns 0") {
    CHECK(fit_headword_size_index("abc", 0, 100, linear_sized_measurer) == 0);
}

TEST_CASE("fit_headword_size_index — empty text fits at largest") {
    CHECK(fit_headword_size_index("", 6, 100, linear_sized_measurer) == 5);
}

TEST_CASE("weather_label — known states map to Swedish") {
    CHECK(weather_label("sunny") == "soligt");
    CHECK(weather_label("rainy") == "regn");
    CHECK(weather_label("partlycloudy") == "växlande");
    CHECK(weather_label("snowy-rainy") == "snöblandat");
    CHECK(weather_label("windy-variant") == "kraftig vind");
    CHECK(weather_label("exceptional") == "extremt");
}

TEST_CASE("weather_label — unknown state passes through unchanged") {
    CHECK(weather_label("not-a-real-state") == "not-a-real-state");
    CHECK(weather_label("") == "");
}

TEST_CASE("pos_long — extracts content inside parens") {
    CHECK(pos_long("no. (nafnorð)") == "nafnorð");
    CHECK(pos_long("lo. (lýsingarorð)") == "lýsingarorð");
    CHECK(pos_long("so. (sagnorð)") == "sagnorð");
}

TEST_CASE("pos_long — no parens falls back to input") {
    CHECK(pos_long("no.") == "no.");
    CHECK(pos_long("") == "");
}

TEST_CASE("pos_long — unclosed paren falls back to input") {
    CHECK(pos_long("no. (oops") == "no. (oops");
}

TEST_CASE("pos_long — empty parens returns empty") {
    CHECK(pos_long("no. ()") == "");
}

// plan_dotted_translations tests use:
//   char_count_measurer (1 unit per byte)
//   sep = " - " (3 bytes wide)
//   line_h = 10, y_start = 0
TEST_CASE("plan_dotted_translations — empty items returns empty plan") {
    auto plan = plan_dotted_translations({}, char_count_measurer, " - ", 3,
                                          20, 10, 0, 1000, false);
    CHECK(plan.full_lines.empty());
    CHECK(plan.trailing_text.empty());
    CHECK(plan.trailing_width == 0);
    CHECK(plan.dynamic_overflow == 0);
}

TEST_CASE("plan_dotted_translations — items joined onto a single trailing line when they fit") {
    // "hi - yo" = 7 bytes, max_width=10. Whole thing rides in trailing.
    auto plan = plan_dotted_translations({"hi", "yo"}, char_count_measurer, " - ", 3,
                                          10, 10, 0, 1000, false);
    CHECK(plan.full_lines.empty());
    CHECK(plan.trailing_text == "hi - yo");
    CHECK(plan.trailing_width == 7);
    CHECK(plan.dynamic_overflow == 0);
}

TEST_CASE("plan_dotted_translations — wraps to multiple lines when joined width exceeds max_width") {
    // 3 items of width 3 each, sep width 3. "abc - def"=9 > 7 forces wrap.
    auto plan = plan_dotted_translations({"abc", "def", "ghi"}, char_count_measurer, " - ", 3,
                                          7, 10, 0, 1000, false);
    REQUIRE(plan.full_lines.size() == 2);
    CHECK(plan.full_lines[0] == "abc");
    CHECK(plan.full_lines[1] == "def");
    CHECK(plan.trailing_text == "ghi");
    CHECK(plan.trailing_width == 3);
    CHECK(plan.dynamic_overflow == 0);
}

TEST_CASE("plan_dotted_translations — vertical limit truncates and counts dynamic overflow") {
    // 5 items, max_width=1 (one per line), line_h=10, y_start=0, max_y_bottom=25.
    // No tag reserve → can fit y=0..15 i.e. 2 lines, push 3 to overflow.
    auto plan = plan_dotted_translations({"a", "b", "c", "d", "e"}, char_count_measurer, " - ", 3,
                                          1, 10, 0, 25, false);
    REQUIRE(plan.full_lines.size() == 2);
    CHECK(plan.full_lines[0] == "a");
    CHECK(plan.full_lines[1] == "b");
    CHECK(plan.trailing_text.empty());  // broke after a buf flush
    CHECK(plan.dynamic_overflow == 3);
}

TEST_CASE("plan_dotted_translations — reserve_for_tag eats one line of capacity") {
    // Same shape as above; with reserve_for_tag=true the planner holds back a
    // line for the caller's "+N fler" tag, so only 1 item makes it to full.
    auto plan = plan_dotted_translations({"a", "b", "c", "d", "e"}, char_count_measurer, " - ", 3,
                                          1, 10, 0, 25, true);
    REQUIRE(plan.full_lines.size() == 1);
    CHECK(plan.full_lines[0] == "a");
    CHECK(plan.dynamic_overflow == 4);
}

TEST_CASE("plan_dotted_translations — oversized item is space-wrapped inline") {
    // Single "hello world" item (11) at max_width=5 → wrap_text_pure yields
    // ["hello", "world"]; "hello" goes to full_lines, "world" becomes trailing.
    auto plan = plan_dotted_translations({"hello world"}, char_count_measurer, " - ", 3,
                                          5, 10, 0, 1000, false);
    REQUIRE(plan.full_lines.size() == 1);
    CHECK(plan.full_lines[0] == "hello");
    CHECK(plan.trailing_text == "world");
    CHECK(plan.trailing_width == 5);
    CHECK(plan.dynamic_overflow == 0);
}

TEST_CASE("plan_dotted_translations — truncates mid-item when internal wrap exceeds y limit") {
    // Single "a b c d e" wraps to 5 sub-lines at max_width=1. With y_start=0,
    // line_h=10, max_y_bottom=25 and no tag reserve: 2 sub-lines fit (y→10,
    // y→20); the third can't. Whole item counts as overflowed; the partial
    // sub-lines already drawn stay in full_lines, trailing remains empty.
    auto plan = plan_dotted_translations({"a b c d e"}, char_count_measurer, " - ", 3,
                                          1, 10, 0, 25, false);
    REQUIRE(plan.full_lines.size() == 2);
    CHECK(plan.full_lines[0] == "a");
    CHECK(plan.full_lines[1] == "b");
    CHECK(plan.trailing_text.empty());
    CHECK(plan.dynamic_overflow == 1);
}
