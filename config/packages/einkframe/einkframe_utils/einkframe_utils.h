#pragma once

#include <functional>
#include <string>
#include <vector>

int minutes_to_next_slot(int current_hour, int current_minute);

float voltage_to_battery_percent(float voltage);

std::string weather_icon(const std::string& state);

std::string ui_icon(const std::string& key);

using TextMeasurer = std::function<int(const std::string&)>;

// When `hard_break` is true and a single token still overflows `max_width`,
// the token is split at UTF-8 char boundaries and broken lines get a trailing
// "-". Default behavior leaves overlong tokens intact on their own line.
std::vector<std::string> wrap_text_pure(const std::string& text, int max_width,
                                         const TextMeasurer& measure,
                                         bool hard_break = false);

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

// Plan for laying out a dotted translation list. `full_lines` are the lines
// already known to be printed verbatim (no tag). `trailing_text` is the last
// partially-filled line; the caller decides whether to inline a "+N fler" tag
// onto it (using `trailing_width` for the fit check) or emit it on its own.
// `dynamic_overflow` counts items dropped because they wouldn't fit before
// `max_y_bottom`. The caller adds this to its own entry-overflow count when
// formatting the tag.
struct TranslationLayout {
    std::vector<std::string> full_lines;
    std::string trailing_text;
    int trailing_width;
    int dynamic_overflow;
};

// Greedy line-fill planner for the dotted translation list. Items are joined
// by `sep` (measured width `sep_w`). Items wider than `max_width` are
// space-wrapped via wrap_text_pure. Layout stops once one more line would push
// `y_start + (n+1) * line_h` past `max_y_bottom`. When `reserve_for_tag` is
// true (or once the planner is forced to truncate), an extra `line_h` is held
// in reserve so the caller has room for a "+N fler" tag line.
TranslationLayout plan_dotted_translations(const std::vector<std::string>& items,
                                            const TextMeasurer& measure,
                                            const std::string& sep,
                                            int sep_w,
                                            int max_width,
                                            int line_h,
                                            int y_start,
                                            int max_y_bottom,
                                            bool reserve_for_tag);

// Extract the spelled-out word class from a `pos` string like "no. (nafnorð)"
// → "nafnorð". Falls back to the input when no parens are present.
std::string pos_long(const std::string& pos);
