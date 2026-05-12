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

// Waveshare 7.5" v2: panel is 800×480 but the usable print area is
// (55,45)..(739,479) inclusive. See specs/einkframe/ARCHITECTURE.md.
// All layout coordinates below are absolute screen coords inside that box.
constexpr int DISPLAY_X1 = 55;
constexpr int DISPLAY_Y1 = 45;
constexpr int DISPLAY_X2 = 739;
constexpr int DISPLAY_Y2 = 479;
constexpr int DISPLAY_W = DISPLAY_X2 - DISPLAY_X1 + 1;  // 685
constexpr int DISPLAY_H = DISPLAY_Y2 - DISPLAY_Y1 + 1;  // 435

// ---- Folio header (top 54 px of usable area) ----
constexpr int FOLIO_H = 54;
constexpr int FOLIO_Y_TOP = DISPLAY_Y1;                          // 45
constexpr int FOLIO_Y_BOTTOM = FOLIO_Y_TOP + FOLIO_H - 1;        // 98
constexpr int FOLIO_HAIRLINE_Y = FOLIO_Y_BOTTOM;                 // 98
constexpr int FOLIO_PAD_X = 32;
constexpr int FOLIO_GAP = 10;

// ---- Body ----
constexpr int BODY_Y_START = FOLIO_Y_TOP + FOLIO_H + 18;         // 117
constexpr int CONTENT_X1 = DISPLAY_X1 + FOLIO_PAD_X;             // 87
constexpr int CONTENT_X2 = DISPLAY_X2 - FOLIO_PAD_X;             // 707
constexpr int CONTENT_W = CONTENT_X2 - CONTENT_X1 + 1;           // 621
constexpr int COL_GAP = 36;
constexpr int LEFT_COL_X = CONTENT_X1;                           // 87
constexpr int LEFT_COL_W = (CONTENT_W - COL_GAP) / 2;            // 292
constexpr int RIGHT_COL_X = LEFT_COL_X + LEFT_COL_W + COL_GAP;   // 415
constexpr int RIGHT_COL_W = CONTENT_X2 - RIGHT_COL_X + 1;        // 293

// ---- Peg labels ----
constexpr int PEG_SQ_SIZE = 7;
constexpr int PEG_GAP = 10;
constexpr int PEG_ROW_H = 14;

// ---- Hero column spacing ----
constexpr int HW_FIT_PAD_RIGHT = 8;
constexpr int CLASS_TOP_MARGIN = 8;
constexpr int INFL_TOP_MARGIN = 2;
constexpr int SE_BLOCK_TOP_MARGIN = 16;
constexpr int TRANS_TOP_MARGIN = 8;

// ---- Examples column ----
constexpr int EX_LIST_TOP_MARGIN = 10;
constexpr int EX_RULE_OFFSET_X = 6;
constexpr int EX_RULE_W = 2;
constexpr int EX_TEXT_INDENT = EX_RULE_OFFSET_X + EX_RULE_W + 14;
constexpr int EX_FIRST_PAD_TOP = 2;
constexpr int EX_PAD = 10;
constexpr int EX_SE_TOP_MARGIN = 3;

// ---- Strip (bottom 60 px of usable area) ----
constexpr int STRIP_H = 60;
constexpr int STRIP_Y = DISPLAY_Y2 - STRIP_H + 1;  // 420 (top row of strip)
constexpr int STRIP_RULE_THICK = 3;
constexpr int STRIP_CELL_W = DISPLAY_W / 4;        // 171
constexpr int STRIP_CELL_PAD_Y = 8;
constexpr int STRIP_VAL_GAP = 6;             // icon → value
constexpr int STRIP_VAL_CENTER_OFFSET = 26;  // lab top → value/icon vertical center

const std::string TRANS_SEP = " · ";

void draw_peg(Display &it, Font *font, int x, int y_top,
              const char *label, bool outline) {
    if (outline) {
        it.rectangle(x, y_top, PEG_SQ_SIZE, PEG_SQ_SIZE);
    } else {
        it.filled_rectangle(x, y_top, PEG_SQ_SIZE, PEG_SQ_SIZE);
    }
    const int text_x = x + PEG_SQ_SIZE + PEG_GAP;
    const int text_y = y_top + PEG_SQ_SIZE / 2;
    it.printf(text_x, text_y, font, TextAlign::CENTER_LEFT, "%s", label);
}

// Wrap `text` for `font` at `max_width` and stamp each line at (x, y),
// advancing y by the font's line height. Safe to call when the text already
// fits on one line: wrap_text returns a single-element vector in that case.
void draw_wrapped(Display &it, Font *font, const std::string &text,
                  int x, int &y, int max_width) {
    std::vector<std::string> lines = wrap_text(font, text, max_width);
    const int line_h = font->get_height();
    for (const auto &line : lines) {
        it.printf(x, y, font, "%s", line.c_str());
        y += line_h;
    }
}

// Greedy line-fill: place dotted translations across multiple lines, wrapping
// individual phrases at space-only boundaries (never mid-token). The "+N fler"
// overflow tag is rendered in Inter via `unit_font` and may share the final
// line with the last translation if it fits.
void draw_dotted_translations(Display &it, Font *trans_font, Font *unit_font,
                              const std::vector<std::string> &items,
                              int overflow_count,
                              int x, int &y, int max_width) {
    if (items.empty() && overflow_count == 0) return;

    const int line_h = trans_font->get_height();
    const int sep_w = get_text_width(trans_font, TRANS_SEP);

    std::string buf;
    int buf_w = 0;

    auto emit_line = [&]() {
        if (!buf.empty()) {
            it.printf(x, y, trans_font, "%s", buf.c_str());
            y += line_h;
            buf.clear();
            buf_w = 0;
        }
    };

    // Replace `buf` with `item` (assumed empty `buf` precondition). If `item`
    // is longer than the column, wrap it at spaces — all sub-lines except the
    // last are emitted directly, and the final sub-line becomes the new buf so
    // a subsequent translation can still join it.
    auto start_with_item = [&](const std::string &item, int item_w) {
        if (item_w <= max_width) {
            buf = item;
            buf_w = item_w;
            return;
        }
        std::vector<std::string> sub_lines = wrap_text(trans_font, item, max_width);
        for (size_t j = 0; j < sub_lines.size(); j++) {
            if (j + 1 == sub_lines.size()) {
                buf = sub_lines[j];
                buf_w = get_text_width(trans_font, sub_lines[j]);
            } else {
                it.printf(x, y, trans_font, "%s", sub_lines[j].c_str());
                y += line_h;
            }
        }
    };

    for (const auto &item : items) {
        const int item_w = get_text_width(trans_font, item);

        if (buf.empty()) {
            start_with_item(item, item_w);
            continue;
        }

        if (buf_w + sep_w + item_w <= max_width) {
            buf += TRANS_SEP + item;
            buf_w = buf_w + sep_w + item_w;
        } else {
            emit_line();
            start_with_item(item, item_w);
        }
    }

    if (overflow_count > 0) {
        char tag_buf[32];
        std::snprintf(tag_buf, sizeof(tag_buf), "+%d fler", overflow_count);
        const std::string tag = tag_buf;
        const int tag_w = get_text_width(unit_font, tag);

        if (!buf.empty()) {
            // Inline tag if it fits on the current line (mixed-font draw).
            if (buf_w + sep_w + tag_w <= max_width) {
                const std::string prefix = buf + TRANS_SEP;
                it.printf(x, y, trans_font, "%s", prefix.c_str());
                const int prefix_w = get_text_width(trans_font, prefix);
                it.printf(x + prefix_w, y, unit_font, "%s", tag.c_str());
                y += line_h;
                buf.clear();
                buf_w = 0;
            } else {
                emit_line();
                it.printf(x, y, unit_font, "%s", tag.c_str());
                y += line_h;
            }
        } else {
            it.printf(x, y, unit_font, "%s", tag.c_str());
            y += line_h;
        }
    }

    emit_line();
}

void draw_hero_column(Display &it, const EntryFonts &fonts, const EntryState &state) {
    int y = BODY_Y_START;

    draw_peg(it, fonts.peg, LEFT_COL_X, y, "ÍSLENSKA", /*outline=*/false);
    y += PEG_ROW_H;

    // Auto-fit headword: pick largest ladder size that fits the column.
    const int hw_max_w = LEFT_COL_W - HW_FIT_PAD_RIGHT;
    const auto measure = [&](int idx, const std::string &t) {
        return get_text_width(fonts.headword_ladder[idx], t);
    };
    const int idx = fit_headword_size_index(state.word,
                                             static_cast<int>(fonts.headword_ladder.size()),
                                             hw_max_w, measure);
    Font *hw_font = fonts.headword_ladder[idx];
    // If even the smallest ladder size overflows, draw_wrapped breaks at
    // spaces (rare edge case). Otherwise wrap_text returns one line.
    draw_wrapped(it, hw_font, state.word, LEFT_COL_X, y, hw_max_w);

    if (!state.pos_long.empty()) {
        y += CLASS_TOP_MARGIN;
        it.printf(LEFT_COL_X, y, fonts.italic_small, "%s", state.pos_long.c_str());
        y += fonts.italic_small->get_height();
    }

    if (!state.inflections.empty()) {
        y += INFL_TOP_MARGIN;
        draw_wrapped(it, fonts.italic_small, state.inflections, LEFT_COL_X, y, LEFT_COL_W);
    }

    y += SE_BLOCK_TOP_MARGIN;
    draw_peg(it, fonts.peg, LEFT_COL_X, y, "SVENSKA", /*outline=*/false);
    y += PEG_ROW_H + TRANS_TOP_MARGIN;

    draw_dotted_translations(it, fonts.translation, fonts.unit,
                             state.translations, state.translation_overflow,
                             LEFT_COL_X, y, LEFT_COL_W);
}

void draw_examples_column(Display &it, const EntryFonts &fonts, const EntryState &state) {
    int y = BODY_Y_START;

    draw_peg(it, fonts.peg, RIGHT_COL_X, y, "DÆMI", /*outline=*/true);
    y += PEG_ROW_H;

    if (state.examples.empty()) return;

    y += EX_LIST_TOP_MARGIN;
    const int list_top = y;
    const int text_x = RIGHT_COL_X + EX_TEXT_INDENT;
    const int text_max_w = RIGHT_COL_W - EX_TEXT_INDENT;
    const int hairline_x1 = RIGHT_COL_X + EX_RULE_OFFSET_X + EX_RULE_W;
    const int hairline_x2 = RIGHT_COL_X + RIGHT_COL_W;

    for (size_t i = 0; i < state.examples.size(); i++) {
        const ExamplePair &ex = state.examples[i];
        y += (i == 0 ? EX_FIRST_PAD_TOP : EX_PAD);

        draw_wrapped(it, fonts.example_is, ex.is, text_x, y, text_max_w);
        y += EX_SE_TOP_MARGIN;
        draw_wrapped(it, fonts.example_se, ex.se, text_x, y, text_max_w);

        if (i + 1 < state.examples.size()) {
            y += EX_PAD;
            it.horizontal_line(hairline_x1, y, hairline_x2 - hairline_x1);
        }
    }
    const int list_bottom = y;

    // Vertical 2 px rule along the full list height.
    it.filled_rectangle(RIGHT_COL_X + EX_RULE_OFFSET_X, list_top,
                        EX_RULE_W, list_bottom - list_top);
}

}  // namespace

void draw_folio_header(Display &it, const FolioFonts &fonts, const FolioHeaderState &state) {
    const int y_center = FOLIO_Y_TOP + FOLIO_H / 2;

    // Left: TITLE · DATE
    int x = CONTENT_X1;
    it.printf(x, y_center, fonts.label, TextAlign::CENTER_LEFT, "%s", state.title.c_str());
    x += get_text_width(fonts.label, state.title) + FOLIO_GAP;
    it.printf(x, y_center, fonts.label, TextAlign::CENTER_LEFT, "·");
    x += get_text_width(fonts.label, "·") + FOLIO_GAP;
    it.printf(x, y_center, fonts.label, TextAlign::CENTER_LEFT, "%s", state.date.c_str());

    // Right: WEATHER ICON + TEMP°
    char temp_buf[16];
    std::snprintf(temp_buf, sizeof(temp_buf), "%s°", state.temperature.c_str());
    it.printf(CONTENT_X2, y_center, fonts.temp, TextAlign::CENTER_RIGHT, "%s", temp_buf);
    const int temp_w = get_text_width(fonts.temp, temp_buf);
    const int icon_x = CONTENT_X2 - temp_w - FOLIO_GAP;
    it.printf(icon_x, y_center, fonts.mdi, TextAlign::CENTER_RIGHT, "%s", state.weather_icon.c_str());

    it.horizontal_line(DISPLAY_X1, FOLIO_HAIRLINE_Y, DISPLAY_W);
}

void draw_entry(Display &it, const EntryFonts &fonts, const EntryState &state) {
    draw_hero_column(it, fonts, state);
    draw_examples_column(it, fonts, state);
}

void draw_strip(Display &it, const StripFonts &fonts, const StripState &state) {
    it.filled_rectangle(DISPLAY_X1, STRIP_Y, DISPLAY_W, STRIP_RULE_THICK);

    const int cells_y = STRIP_Y + STRIP_RULE_THICK;
    const int lab_y = cells_y + STRIP_CELL_PAD_Y;
    // CENTER alignment for icon + value + unit so MDI glyphs (which have no
    // typographic baseline) line up with text by visual bbox center.
    const int val_center_y = lab_y + STRIP_VAL_CENTER_OFFSET;

    for (size_t i = 0; i < state.cells.size() && i < 4; i++) {
        const StripCell &c = state.cells[i];
        const int cell_x = DISPLAY_X1 + static_cast<int>(i) * STRIP_CELL_W;
        const int cell_center_x = cell_x + STRIP_CELL_W / 2;

        if (i > 0) {
            it.vertical_line(cell_x, cells_y + 4, STRIP_H - STRIP_RULE_THICK - 8);
        }

        // Label: horizontally centered above the value row.
        it.printf(cell_center_x, lab_y, fonts.label, TextAlign::TOP_CENTER, "%s", c.label.c_str());

        // Value row: pre-measure to center icon + value + unit as a block.
        const int icon_w = c.icon.empty() ? 0 : get_text_width(fonts.mdi, c.icon);
        const int value_w = get_text_width(fonts.value, c.value);
        const int unit_w = c.unit.empty() ? 0 : get_text_width(fonts.unit, c.unit);
        const int icon_gap = (icon_w > 0) ? STRIP_VAL_GAP : 0;
        const int total_w = icon_w + icon_gap + value_w + unit_w;
        int x = cell_center_x - total_w / 2;

        if (!c.icon.empty()) {
            it.printf(x, val_center_y, fonts.mdi, TextAlign::CENTER_LEFT, "%s", c.icon.c_str());
            x += icon_w + STRIP_VAL_GAP;
        }
        it.printf(x, val_center_y, fonts.value, TextAlign::CENTER_LEFT, "%s", c.value.c_str());
        if (!c.unit.empty()) {
            x += value_w;
            it.printf(x, val_center_y, fonts.unit, TextAlign::CENTER_LEFT, "%s", c.unit.c_str());
        }
    }
}
