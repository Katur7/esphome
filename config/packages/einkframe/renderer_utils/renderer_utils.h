#pragma once

#include <string>

#include "esphome/components/display/display.h"
#include "esphome/components/font/font.h"

struct TitleState {
    std::string date;
};

struct InfoColumnState {
    std::string weather;
    std::string temperature;
    std::string expected_rain;
    std::string uv_index;
    float battery_percent;
};

struct WordOfDayState {
    std::string word;
    std::string translations;
    std::string example1_is;
    std::string example1_se;
    std::string example2_is;
    std::string example2_se;
};

void draw_title(esphome::display::Display &it,
                esphome::font::Font *title_font,
                const TitleState &state);

void draw_info_column(esphome::display::Display &it,
                      esphome::font::Font *subheading_font,
                      esphome::font::Font *normal_font,
                      esphome::font::Font *mdi_font,
                      const InfoColumnState &state);

void draw_word_of_day(esphome::display::Display &it,
                      esphome::font::Font *subheading_font,
                      esphome::font::Font *normal_font,
                      esphome::font::Font *smaller_font,
                      const WordOfDayState &state);
