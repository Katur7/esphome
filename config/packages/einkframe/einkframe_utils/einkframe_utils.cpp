#include "einkframe_utils.h"

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

std::vector<std::string> wrap_text_pure(const std::string& text, int max_width, const TextMeasurer& measure) {
    std::vector<std::string> wrapped_lines;
    if (text.empty()) {
        return wrapped_lines;
    }

    if (measure(text) <= max_width) {
        wrapped_lines.push_back(text);
        return wrapped_lines;
    }

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
            }
            current_line = word;
        }
    }

    if (!current_line.empty()) {
        wrapped_lines.push_back(current_line);
    }
    return wrapped_lines;
}
