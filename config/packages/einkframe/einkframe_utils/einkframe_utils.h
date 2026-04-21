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
