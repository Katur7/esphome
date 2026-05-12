#pragma once

#include <string>
#include <vector>

#include "esphome/components/display/display.h"
#include "esphome/components/font/font.h"

// =============================================================================
// 07a layout — three panel functions (folio header, entry, strip).
// Dimensions and visual hierarchy described in:
//   specs/einkframe/tasks/08-design-rewamp/DESIGN.md
//   specs/einkframe/tasks/08-design-rewamp/preview/styles.css (.v-ord-a)
// =============================================================================

struct ExamplePair {
    std::string is;
    std::string se;
};

struct FolioHeaderState {
    std::string title;         // "ORÐ DAGSINS"
    std::string date;
    std::string weather_icon;  // MDI glyph string
    std::string temperature;   // sans degree sign — renderer appends it
};

struct FolioFonts {
    esphome::font::Font *label;
    esphome::font::Font *temp;
    esphome::font::Font *mdi;
};

void draw_folio_header(esphome::display::Display &it,
                       const FolioFonts &fonts,
                       const FolioHeaderState &state);

struct EntryState {
    std::string word;
    std::string pos_long;                     // already extracted from "no. (nafnorð)"
    std::string inflections;                  // empty if absent → line is hidden
    std::vector<std::string> translations;    // already split + capped
    int translation_overflow;                 // 0 if not capped; else N for "+N fler"
    std::vector<ExamplePair> examples;
};

// Headword ladder: largest size first preferred. fit_headword_size_index walks
// the ladder largest-to-smallest and picks the first size that fits. The vector
// must be ordered smallest → largest (mirrors the einkframe_utils contract).
struct EntryFonts {
    std::vector<esphome::font::Font*> headword_ladder;
    esphome::font::Font *peg;
    esphome::font::Font *italic_small;
    esphome::font::Font *translation;
    esphome::font::Font *unit;          // "+N fler" tag
    esphome::font::Font *example_is;
    esphome::font::Font *example_se;
};

void draw_entry(esphome::display::Display &it,
                const EntryFonts &fonts,
                const EntryState &state);

struct StripCell {
    std::string label;
    std::string icon;   // MDI glyph; empty = no icon
    std::string value;
    std::string unit;   // e.g. "%"; empty = no unit
};

struct StripState {
    std::vector<StripCell> cells;  // expected 4
};

struct StripFonts {
    esphome::font::Font *label;
    esphome::font::Font *value;
    esphome::font::Font *unit;
    esphome::font::Font *mdi;
};

void draw_strip(esphome::display::Display &it,
                const StripFonts &fonts,
                const StripState &state);
