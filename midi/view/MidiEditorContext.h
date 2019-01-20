#pragma once

#include <memory>

class MidiViewport;
class MidiSong;

class MidiEditorContext
{
public:
    MidiEditorContext(std::shared_ptr<MidiSong>);
    ~MidiEditorContext();

    // TODO: don't allow direct access?
    std::shared_ptr<MidiViewport> viewport;
    float cursorTime = 0;
    float cursorPitch = 0;
};

using MidiEditorContextPtr = std::shared_ptr<MidiEditorContext>;