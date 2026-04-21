#include "renderer_utils.h"

#include <cstdlib>
#include <vector>

#include "../einkframe_utils/einkframe_utils.h"
#include "../text_utils/text_utils.h"

using esphome::display::Display;
using esphome::display::TextAlign;
using esphome::font::Font;

void draw_title(Display &it, Font *title_font, const TitleState &state) {
    it.printf(397, 60, title_font, TextAlign::TOP_CENTER, "%s", state.date.c_str());
}

void draw_info_column(Display &it,
                      Font *subheading_font,
                      Font *normal_font,
                      Font *mdi_font,
                      const InfoColumnState &state) {
    it.printf(100, 110, subheading_font, "Vädret idag");

    it.printf(130, 160, mdi_font, TextAlign::CENTER_RIGHT, "%s", weather_icon(state.weather).c_str());
    it.printf(140, 160, normal_font, TextAlign::CENTER_LEFT, "%s ˚C", state.temperature.c_str());

    it.printf(130, 200, mdi_font, TextAlign::CENTER_RIGHT, "%s", ui_icon("mdi-cloud-percent").c_str());
    float rain_val = atof(state.expected_rain.c_str());
    it.printf(140, 200, normal_font, TextAlign::CENTER_LEFT, "%.1f%%", rain_val);
    int expected_rain_length = get_text_width(normal_font, state.expected_rain);
    it.printf(140 + expected_rain_length + 40, 200, mdi_font, TextAlign::CENTER_LEFT, "%s", ui_icon("mdi-umbrella").c_str());

    it.printf(130, 250, mdi_font, TextAlign::CENTER_RIGHT, "%s", ui_icon("mdi-sun-wireless-outline").c_str());
    it.printf(140, 250, normal_font, TextAlign::CENTER_LEFT, "%.1f", atof(state.uv_index.c_str()));

    it.printf(130, 300, mdi_font, TextAlign::CENTER_RIGHT, "%s", ui_icon("mdi-battery").c_str());
    it.printf(140, 300, normal_font, TextAlign::CENTER_LEFT, "%.0f%%", state.battery_percent);
}

void draw_word_of_day(Display &it,
                      Font *subheading_font,
                      Font *normal_font,
                      Font *smaller_font,
                      const WordOfDayState &state) {
    it.printf(300, 110, subheading_font, "Dagens ord");
    it.printf(300, 150, normal_font, "%s", state.word.c_str());

    const std::string se_prefix = "  - ";
    int translation_prefix_length = get_text_width(normal_font, se_prefix);
    int current_line_y = 185;

    std::vector<std::string> wrapped_translations = wrap_text(normal_font, state.translations, 400);
    it.printf(300, current_line_y, normal_font, "%s", se_prefix.c_str());
    for (const auto &line : wrapped_translations) {
        it.printf(300 + translation_prefix_length, current_line_y, normal_font, "%s", line.c_str());
        current_line_y += 25;
    }
    current_line_y += 15;

    int example_prefix_length = get_text_width(smaller_font, se_prefix);

    std::vector<std::string> wrapped_example1_is = wrap_text(smaller_font, state.example1_is, 400);
    for (const auto &line : wrapped_example1_is) {
        it.printf(300, current_line_y, smaller_font, "%s", line.c_str());
        current_line_y += 25;
    }
    std::vector<std::string> wrapped_example1_se = wrap_text(smaller_font, state.example1_se, 400);
    it.printf(300, current_line_y, smaller_font, "%s", se_prefix.c_str());
    for (const auto &line : wrapped_example1_se) {
        it.printf(300 + example_prefix_length, current_line_y, smaller_font, "%s", line.c_str());
        current_line_y += 25;
    }

    if (state.example2_is != "unknown") {
        std::vector<std::string> wrapped_example2_is = wrap_text(smaller_font, state.example2_is, 400);
        for (const auto &line : wrapped_example2_is) {
            it.printf(300, current_line_y, smaller_font, "%s", line.c_str());
            current_line_y += 25;
        }
        std::vector<std::string> wrapped_example2_se = wrap_text(smaller_font, state.example2_se, 400);
        it.printf(300, current_line_y, smaller_font, "%s", se_prefix.c_str());
        for (const auto &line : wrapped_example2_se) {
            it.printf(300 + example_prefix_length, current_line_y, smaller_font, "%s", line.c_str());
            current_line_y += 25;
        }
    }
}
