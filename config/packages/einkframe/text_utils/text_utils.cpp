#include <vector>
#include <string>


#include "text_utils.h"
#include "../einkframe_utils/einkframe_utils.h"
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
    if (font == nullptr) {
        return {};
    }
    return wrap_text_pure(text, max_width, [font](const std::string& s) {
        return get_text_width(font, s);
    });
}
