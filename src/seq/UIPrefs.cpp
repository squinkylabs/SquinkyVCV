#include "UIPrefs.h"


//const NVGcolor UIPrefs::VU_ACTIVE_COLOR = nvgRGB(0, 0xc0, 0);
const NVGcolor UIPrefs::VU_ACTIVE_COLOR = nvgRGB(48, 125, 255);

const NVGcolor UIPrefs::VU_INACTIVE_COLOR = nvgRGB(24, 66, 128);
//const NVGcolor UIPrefs::VU_INACTIVE_COLOR = nvgRGB(0, 0x70, 0);

const NVGcolor UIPrefs::NOTE_COLOR =
nvgRGB(0, 0xc0, 0);
const NVGcolor UIPrefs::SELECTED_NOTE_COLOR =
nvgRGB(0xf0, 0xf0, 0);
const NVGcolor UIPrefs::DRAG_TEXT_COLOR =
nvgRGB(0x20, 0xff, 0x20);                        // same a note color, but brighter.

const NVGcolor UIPrefs::NOTE_EDIT_BACKGROUND =
nvgRGB(0x28, 0x28, 0x2b);
const NVGcolor UIPrefs::NOTE_EDIT_ACCIDENTAL_BACKGROUND =
nvgRGB(0, 0, 0);

const NVGcolor UIPrefs::DRAGGED_NOTE_COLOR =
nvgRGBA(0xff, 0x80, 0, 0x80);

// was 90 / 40
const NVGcolor UIPrefs::GRID_COLOR     = nvgRGB(0x80, 0x80, 0x80);
const NVGcolor UIPrefs::GRID_BAR_COLOR = nvgRGB(0xe0, 0xe0, 0xe0);
const NVGcolor UIPrefs::GRID_END_COLOR = nvgRGB(0xff, 0x0, 0xff);

// was b0
const NVGcolor UIPrefs::TIME_LABEL_COLOR = nvgRGB(0xe0, 0xe0, 0xe0);
const NVGcolor UIPrefs::GRID_CLINE_COLOR = nvgRGB(0x60, 0x60, 0x60);
const NVGcolor UIPrefs::STATUS_LABEL_COLOR = nvgRGB(0xe0, 0xe0, 0xe0);

const NVGcolor UIPrefs::XFORM_TEXT_COLOR = nvgRGB(0xc0, 0xc0, 0xc0);

const NVGcolor UIPrefs::X4_SELECTION_COLOR = nvgRGB(0x0, 0x0, 0x0);

const NVGcolor UIPrefs::X4_BUTTON_FACE_NORM = nvgRGB(0, 0x40, 0);
const NVGcolor UIPrefs::X4_BUTTON_FACE_PLAYING = nvgRGB(0, 0x90, 0);
const NVGcolor UIPrefs::X4_BUTTON_FACE_NONOTES = nvgRGB(0x20, 0x20, 0x20);
const NVGcolor UIPrefs::X4_BUTTON_FACE_NONOTES_PLAYING = nvgRGB(0x40, 0x40, 0x40);
const NVGcolor UIPrefs::X4_BUTTON_FACE_SELECTED =  nvgRGB(0x50, 0, 0x50);

const NVGcolor UIPrefs::X4_SELECTED_BORDER = nvgRGBA(0x80, 0x80, 0x80, 0x80);
const NVGcolor UIPrefs::X4_NEXT_PLAY_BORDER = nvgRGB(0xe0, 0xe0, 0xe0);
const NVGcolor UIPrefs::X4_MIXED_BORDER = nvgRGB(0xc0, 0, 0);
const float UIPrefs::hMarginsNoteEdit = 2.f;
const float UIPrefs::topMarginNoteEdit = 0.f;
const float UIPrefs::timeLabelFontSize = 12.f;

