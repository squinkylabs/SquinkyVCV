#pragma once

#include <memory>

class MidiViewport;

class MidiEditorContext
{
public:
    MidiEditorContext();
    ~MidiEditorContext();

    // TODO: don't allow direct access
    std::shared_ptr<MidiViewport> viewport;
private:
};

using MidiEditorContextPtr = std::shared_ptr<MidiEditorContext>;