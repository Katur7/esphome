#include "einkframe_utils.h"

#include <cctype>
#include <cmath>
#include <map>
#include <sstream>

int minutes_to_next_slot(int current_hour, int current_minute) {
    static const int slots[] = {7, 11, 15, 19};
    static const int num_slots = 4;

    const int current_minutes = current_hour * 60 + current_minute;

    for (int i = 0; i < num_slots; i++) {
        const int slot_minutes = slots[i] * 60;
        if (slot_minutes > current_minutes) {
            return slot_minutes - current_minutes;
        }
    }
    return (24 * 60 - current_minutes) + slots[0] * 60;
}

float voltage_to_battery_percent(float voltage) {
    if (voltage <= 0) return NAN;

    if (voltage > 4.14f) voltage = 4.14f;
    if (voltage < 3.09f) voltage = 3.09f;

    if (voltage >= 4.13f) return 100;
    if (voltage >= 4.11f) return 95;
    if (voltage >= 4.08f) return 90;
    if (voltage >= 4.05f) return 85;
    if (voltage >= 4.02f) return 80;
    if (voltage >= 4.00f) return 75;
    if (voltage >= 3.98f) return 70;
    if (voltage >= 3.96f) return 65;
    if (voltage >= 3.93f) return 60;
    if (voltage >= 3.90f) return 55;
    if (voltage >= 3.87f) return 50;
    if (voltage >= 3.84f) return 45;
    if (voltage >= 3.80f) return 40;
    if (voltage >= 3.77f) return 35;
    if (voltage >= 3.72f) return 30;
    if (voltage >= 3.68f) return 25;
    if (voltage >= 3.64f) return 20;
    if (voltage >= 3.61f) return 15;
    if (voltage >= 3.56f) return 10;
    if (voltage >= 3.49f) return 5;
    return 0;
}

std::string weather_icon(const std::string& state) {
    static const std::map<std::string, std::string> table = {
        {"clear-night",     "\U000F0594"},
        {"cloudy",          "\U000F0590"},
        {"exceptional",     "\U000F05D6"},
        {"fog",             "\U000F0591"},
        {"hail",            "\U000F0592"},
        {"lightning",       "\U000F0593"},
        {"lightning-rainy", "\U000F067E"},
        {"partlycloudy",    "\U000F0595"},
        {"pouring",         "\U000F0596"},
        {"rainy",           "\U000F0597"},
        {"snowy",           "\U000F0598"},
        {"snowy-rainy",     "\U000F067F"},
        {"sunny",           "\U000F0599"},
        {"windy",           "\U000F059D"},
        {"windy-variant",   "\U000F059E"},
    };
    auto it = table.find(state);
    return it == table.end() ? "" : it->second;
}

std::string ui_icon(const std::string& key) {
    static const std::map<std::string, std::string> table = {
        {"mdi-cloud-percent",        "\U000F1A35"},
        {"mdi-umbrella",             "\U000F054A"},
        {"mdi-sun-wireless-outline", "\U000F17FF"},
        {"mdi-battery",              "\U000F0079"},
    };
    auto it = table.find(key);
    return it == table.end() ? "" : it->second;
}

namespace {

std::string trim_ws(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(start, end - start);
}

}  // namespace

std::vector<std::string> split_translations(const std::string& s) {
    std::vector<std::string> result;
    std::string current;
    size_t i = 0;
    while (i < s.size()) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        size_t sep_len = 0;
        if (c == ',') {
            sep_len = 1;
        } else if (c == 0xC2 && i + 1 < s.size() &&
                   static_cast<unsigned char>(s[i + 1]) == 0xB7) {
            sep_len = 2;  // UTF-8 for U+00B7 (·)
        }
        if (sep_len > 0) {
            std::string trimmed = trim_ws(current);
            if (!trimmed.empty()) result.push_back(trimmed);
            current.clear();
            i += sep_len;
        } else {
            current.push_back(s[i]);
            i++;
        }
    }
    std::string trimmed = trim_ws(current);
    if (!trimmed.empty()) result.push_back(trimmed);
    return result;
}

CappedTranslations cap_translations(const std::vector<std::string>& items, int max_visible) {
    if (max_visible < 0) max_visible = 0;
    if (static_cast<int>(items.size()) <= max_visible) {
        return {items, 0};
    }
    CappedTranslations result;
    result.visible.assign(items.begin(), items.begin() + max_visible);
    result.overflow = static_cast<int>(items.size()) - max_visible;
    return result;
}

int fit_headword_size_index(const std::string& text, int num_sizes, int max_width,
                             const SizedTextMeasurer& measure) {
    if (num_sizes <= 0) return 0;
    for (int i = num_sizes - 1; i >= 0; i--) {
        if (measure(i, text) <= max_width) return i;
    }
    return 0;
}

std::string weather_label(const std::string& state) {
    static const std::map<std::string, std::string> table = {
        {"clear-night",     "klart"},
        {"cloudy",          "molnigt"},
        {"exceptional",     "extremt"},
        {"fog",             "dimma"},
        {"hail",            "hagel"},
        {"lightning",       "åska"},
        {"lightning-rainy", "åskregn"},
        {"partlycloudy",    "växlande"},
        {"pouring",         "ösregn"},
        {"rainy",           "regn"},
        {"snowy",           "snö"},
        {"snowy-rainy",     "snöblandat"},
        {"sunny",           "soligt"},
        {"windy",           "blåsigt"},
        {"windy-variant",   "kraftig vind"},
    };
    auto it = table.find(state);
    return it == table.end() ? state : it->second;
}

TranslationLayout plan_dotted_translations(const std::vector<std::string>& items,
                                            const TextMeasurer& measure,
                                            const std::string& sep,
                                            int sep_w,
                                            int max_width,
                                            int line_h,
                                            int y_start,
                                            int max_y_bottom,
                                            bool reserve_for_tag) {
    TranslationLayout out;
    out.trailing_width = 0;
    out.dynamic_overflow = 0;

    if (items.empty()) return out;

    std::string buf;
    int buf_w = 0;
    int y = y_start;
    bool truncated = false;

    auto will_have_overflow = [&]() {
        return reserve_for_tag || truncated;
    };

    auto line_fits = [&]() {
        const int reserve = will_have_overflow() ? line_h : 0;
        return y + line_h + reserve <= max_y_bottom;
    };

    auto flush_buf = [&]() {
        if (!buf.empty()) {
            out.full_lines.push_back(buf);
            y += line_h;
            buf.clear();
            buf_w = 0;
        }
    };

    auto start_with_item = [&](const std::string& item, int item_w) -> bool {
        if (item_w <= max_width) {
            buf = item;
            buf_w = item_w;
            return true;
        }
        std::vector<std::string> sub_lines = wrap_text_pure(item, max_width, measure);
        for (size_t j = 0; j < sub_lines.size(); j++) {
            if (j + 1 == sub_lines.size()) {
                buf = sub_lines[j];
                buf_w = measure(sub_lines[j]);
            } else {
                if (!line_fits()) return false;
                out.full_lines.push_back(sub_lines[j]);
                y += line_h;
            }
        }
        return true;
    };

    size_t i = 0;
    for (; i < items.size(); i++) {
        const std::string& item = items[i];
        const int item_w = measure(item);

        if (buf.empty()) {
            if (!line_fits()) { truncated = true; break; }
            if (!start_with_item(item, item_w)) { truncated = true; break; }
            continue;
        }

        if (buf_w + sep_w + item_w <= max_width) {
            buf += sep + item;
            buf_w = buf_w + sep_w + item_w;
        } else {
            if (!line_fits()) { truncated = true; break; }
            flush_buf();
            if (!line_fits()) { truncated = true; break; }
            if (!start_with_item(item, item_w)) { truncated = true; break; }
        }
    }
    out.dynamic_overflow = static_cast<int>(items.size() - i);

    out.trailing_text = buf;
    out.trailing_width = buf_w;
    return out;
}

std::string pos_long(const std::string& pos) {
    auto open = pos.find('(');
    if (open == std::string::npos) return pos;
    auto close = pos.find(')', open);
    if (close == std::string::npos) return pos;
    return pos.substr(open + 1, close - open - 1);
}

std::vector<std::string> wrap_text_pure(const std::string& text, int max_width,
                                         const TextMeasurer& measure,
                                         bool hard_break) {
    std::vector<std::string> wrapped_lines;
    if (text.empty()) {
        return wrapped_lines;
    }

    if (measure(text) <= max_width) {
        wrapped_lines.push_back(text);
        return wrapped_lines;
    }

    auto utf8_step = [](unsigned char b) -> size_t {
        if (b < 0x80) return 1;
        if ((b & 0xE0) == 0xC0) return 2;
        if ((b & 0xF0) == 0xE0) return 3;
        if ((b & 0xF8) == 0xF0) return 4;
        return 1;
    };

    // Char-split a single oversized word, pushing hyphenated sub-lines and
    // returning the trailing partial (no hyphen) so it can keep accumulating.
    auto hard_break_word = [&](const std::string& word) -> std::string {
        std::string cur;
        size_t pos = 0;
        while (pos < word.size()) {
            const size_t step = utf8_step(static_cast<unsigned char>(word[pos]));
            const std::string ch = word.substr(pos, step);
            const bool is_last = (pos + step) >= word.size();
            const std::string trial = is_last ? (cur + ch) : (cur + ch + "-");
            if (measure(trial) <= max_width) {
                cur += ch;
                pos += step;
            } else if (cur.empty()) {
                // Single glyph + hyphen still overflows: emit the glyph alone.
                wrapped_lines.push_back(ch);
                pos += step;
            } else {
                wrapped_lines.push_back(cur + "-");
                cur.clear();
            }
        }
        return cur;
    };

    std::istringstream iss(text);
    std::string word;
    std::string current_line;

    while (iss >> word) {
        std::string test_line = current_line.empty() ? word : current_line + " " + word;
        if (measure(test_line) <= max_width) {
            current_line = test_line;
        } else {
            if (!current_line.empty()) {
                wrapped_lines.push_back(current_line);
                current_line.clear();
            }
            if (hard_break && measure(word) > max_width) {
                current_line = hard_break_word(word);
            } else {
                current_line = word;
            }
        }
    }

    if (!current_line.empty()) {
        wrapped_lines.push_back(current_line);
    }
    return wrapped_lines;
}
