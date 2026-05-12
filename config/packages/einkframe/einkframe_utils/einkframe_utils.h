#pragma once

#include <functional>
#include <string>
#include <vector>

int minutes_to_next_slot(int current_hour, int current_minute);

float voltage_to_battery_percent(float voltage);

std::string weather_icon(const std::string& state);

std::string ui_icon(const std::string& key);

using TextMeasurer = std::function<int(const std::string&)>;

std::vector<std::string> wrap_text_pure(const std::string& text, int max_width, const TextMeasurer& measure);

// Split a translation string on `,` or `·` (middle dot, U+00B7).
// Trims whitespace around each item and drops empties.
std::vector<std::string> split_translations(const std::string& s);

// Cap a translation list to at most `max_visible` items. Overflow count is
// what the "+N fler" tag should display. If items.size() <= max_visible the
// full list is returned and overflow is 0.
struct CappedTranslations {
    std::vector<std::string> visible;
    int overflow;
};
CappedTranslations cap_translations(const std::vector<std::string>& items, int max_visible);

// Pick the largest size index from a discrete font ladder such that the text
// fits within max_width. Indices are assumed sorted ascending (smallest first).
// `measure(i, text)` returns the rendered pixel width at ladder index `i`.
// If no size fits, returns 0 (the smallest); the caller must then wrap the
// text. This mirrors useFitHeadword() in the web reference, adapted to
// pre-baked bitmap fonts.
using SizedTextMeasurer = std::function<int(int /*size_index*/, const std::string&)>;
int fit_headword_size_index(const std::string& text, int num_sizes, int max_width,
                             const SizedTextMeasurer& measure);

// Map a Home Assistant weather state to its Swedish display word. Unknown
// states pass through unchanged (mirrors the JS fallback in variant-swiss.jsx).
std::string weather_label(const std::string& state);

// Extract the spelled-out word class from a `pos` string like "no. (nafnorð)"
// → "nafnorð". Falls back to the input when no parens are present.
std::string pos_long(const std::string& pos);
