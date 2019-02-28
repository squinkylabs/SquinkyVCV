#pragma once

#include "nanovg.h"
class UIPrefs
{
public:
    static const NVGcolor NOTE_COLOR;
    static const NVGcolor SELECTED_NOTE_COLOR;
    static const NVGcolor NOTE_EDIT_BACKGROUND;
    static const NVGcolor NOTE_EDIT_ACCIDENTAL_BACKGROUND;
    static const NVGcolor GRID_COLOR;
    static const NVGcolor GRID_BAR_COLOR;
    static const NVGcolor TIME_LABEL_COLOR;

    static constexpr float hMarginsNoteEdit = 2.f;
    static constexpr float topMarginNoteEdit = 0.f;
    static constexpr float timeLabelFontSize = 11.f;

};
