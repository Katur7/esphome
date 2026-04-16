#include <vector>
#include <string>
#include <sstream>


#include "text_utils.h"
#include "esphome/core/log.h"
#include "esphome/components/font/font.h"

static const char *TAG = "text_utils";

int get_text_width(esphome::font::Font *font, std::string text) {
    // Safety guard: font must not be null
    if (font == nullptr) {
        ESP_LOGW(TAG, "get_text_width() called with NULL font");
        return 0;
    }

    // Safety guard: text must not be empty
    if (text.empty()) {
        ESP_LOGW(TAG, "get_text_width() called with empty text");
        return 0;
    }

    // Optional: guard against empty strings (not required, but useful)
    if (text[0] == '\0') {
        return 0;
    }

    int width = 0;
    int x_offset = 0;
    int baseline = 0;
    int height = 0;

    font->measure(text.c_str(), &width, &x_offset, &baseline, &height);

    return width;
}

std::vector<std::string> wrap_text(esphome::font::Font *font, std::string text, int max_width) {
    std::vector<std::string> wrapped_lines = {};
    std::string current_line;
    if (font == nullptr || text.empty()) {
        return wrapped_lines;
    }


    if (get_text_width(font, text) <= max_width) {
        wrapped_lines.push_back(std::string(text));
        return wrapped_lines;
    }
    std::istringstream iss(text);
    std::string word;

    while (iss >> word) {
        std::string test_line = current_line.empty() ? word : current_line + " " + word;
        int line_width = get_text_width(font, test_line.c_str());

        if (line_width <= max_width) {
            current_line = test_line;
        } else {
            wrapped_lines.push_back(current_line);
            current_line = word;
        }
    }

    if (!current_line.empty()) {
        wrapped_lines.push_back(current_line);
    }
    return wrapped_lines;
}
