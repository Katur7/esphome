#include "renderer_utils.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "../einkframe_utils/einkframe_utils.h"
#include "../text_utils/text_utils.h"

using esphome::display::Display;
using esphome::display::TextAlign;
using esphome::font::Font;

namespace {

constexpr int TITLE_X = 397;
constexpr int TITLE_Y = 60;

constexpr int INFO_HEADING_X = 100;
constexpr int INFO_HEADING_Y = 110;
constexpr int INFO_COL_ICON_X = 130;
constexpr int INFO_COL_VALUE_X = 140;
constexpr int UMBRELLA_GAP = 40;

constexpr int ROW_WEATHER_Y = 160;
constexpr int ROW_RAIN_Y = 200;
constexpr int ROW_UV_Y = 250;
constexpr int ROW_BATTERY_Y = 300;

constexpr int WORD_COL_X = 300;
constexpr int WORD_HEADING_Y = 110;
constexpr int WORD_Y = 150;
constexpr int TRANSLATIONS_START_Y = 185;

constexpr int LINE_HEIGHT = 25;
constexpr int TRANSLATION_GAP = 15;
constexpr int WRAP_WIDTH = 400;

const std::string SE_PREFIX = "  - ";

std::string format_number(const char *fmt, float value) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), fmt, value);
    return std::string(buf);
}

void draw_info_row(Display &it, int y, Font *mdi_font, const std::string &icon,
                   Font *value_font, const std::string &value_text) {
    it.printf(INFO_COL_ICON_X, y, mdi_font, TextAlign::CENTER_RIGHT, "%s", icon.c_str());
    it.printf(INFO_COL_VALUE_X, y, value_font, TextAlign::CENTER_LEFT, "%s", value_text.c_str());
}

void draw_wrapped_block(Display &it, Font *font, const std::string &text, int x, int &y, int max_width) {
    std::vector<std::string> lines = wrap_text(font, text, max_width);
    for (const auto &line : lines) {
        it.printf(x, y, font, "%s", line.c_str());
        y += LINE_HEIGHT;
    }
}

void draw_prefixed_wrapped_block(Display &it, Font *font, const std::string &text, int x, int &y, int max_width) {
    int prefix_width = get_text_width(font, SE_PREFIX);
    std::vector<std::string> lines = wrap_text(font, text, max_width);
    it.printf(x, y, font, "%s", SE_PREFIX.c_str());
    for (const auto &line : lines) {
        it.printf(x + prefix_width, y, font, "%s", line.c_str());
        y += LINE_HEIGHT;
    }
}

}  // namespace

void draw_title(Display &it, Font *title_font, const TitleState &state) {
    it.printf(TITLE_X, TITLE_Y, title_font, TextAlign::TOP_CENTER, "%s", state.date.c_str());
}

void draw_info_column(Display &it,
                      Font *subheading_font,
                      Font *normal_font,
                      Font *mdi_font,
                      const InfoColumnState &state) {
    it.printf(INFO_HEADING_X, INFO_HEADING_Y, subheading_font, "Vädret idag");

    draw_info_row(it, ROW_WEATHER_Y, mdi_font, weather_icon(state.weather),
                  normal_font, state.temperature + " ˚C");

    float rain_val = std::atof(state.expected_rain.c_str());
    draw_info_row(it, ROW_RAIN_Y, mdi_font, ui_icon("mdi-cloud-percent"),
                  normal_font, format_number("%.1f%%", rain_val));
    int expected_rain_length = get_text_width(normal_font, state.expected_rain);
    it.printf(INFO_COL_VALUE_X + expected_rain_length + UMBRELLA_GAP, ROW_RAIN_Y,
              mdi_font, TextAlign::CENTER_LEFT, "%s", ui_icon("mdi-umbrella").c_str());

    draw_info_row(it, ROW_UV_Y, mdi_font, ui_icon("mdi-sun-wireless-outline"),
                  normal_font, format_number("%.1f", std::atof(state.uv_index.c_str())));

    draw_info_row(it, ROW_BATTERY_Y, mdi_font, ui_icon("mdi-battery"),
                  normal_font, format_number("%.0f%%", state.battery_percent));
}

void draw_word_of_day(Display &it,
                      Font *subheading_font,
                      Font *normal_font,
                      Font *smaller_font,
                      const WordOfDayState &state) {
    it.printf(WORD_COL_X, WORD_HEADING_Y, subheading_font, "Dagens ord");
    it.printf(WORD_COL_X, WORD_Y, normal_font, "%s", state.word.c_str());

    int y = TRANSLATIONS_START_Y;
    draw_prefixed_wrapped_block(it, normal_font, state.translations, WORD_COL_X, y, WRAP_WIDTH);
    y += TRANSLATION_GAP;

    for (const auto &example : state.examples) {
        draw_wrapped_block(it, smaller_font, example.is, WORD_COL_X, y, WRAP_WIDTH);
        draw_prefixed_wrapped_block(it, smaller_font, example.se, WORD_COL_X, y, WRAP_WIDTH);
    }
}
