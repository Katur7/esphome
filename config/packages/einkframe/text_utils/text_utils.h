#pragma once
#include "esphome/components/font/font.h"

int get_text_width(esphome::font::Font *font, std::string text);
std::vector<std::string> wrap_text(esphome::font::Font *font, std::string text, int max_width,
                                    bool hard_break = false);
