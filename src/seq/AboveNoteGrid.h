

#include "../Squinky.hpp"
#include "widget/Widget.hpp"
#include "MidiEditorContext.h"

class MidiSequencer;
class MidiSong;
class SubrangeLoop;
using  MidiSequencerPtr = std::shared_ptr<MidiSequencer>;

struct AboveNoteGrid : OpaqueWidget
{
public:
    AboveNoteGrid(const Vec& pos, const Vec& size, MidiSequencerPtr seq);

    /**
     * Inject a new sequencer into this editor.
     */
    void setSequencer(MidiSequencerPtr seq);
    void songUpdated();

    void draw(const DrawArgs &args) override;
    void step() override;

private:
    bool firstTime = true;
    int curFirstBar = -1;      // number of measure at start of grid
    float curCursorTime = -1;
    float curCursorPitch = -1;
    std::shared_ptr<SubrangeLoop> curLoop;
    MidiSequencerPtr sequencer;
    Label* editAttributeLabel = nullptr;
    MidiEditorContext::NoteAttribute curAttribute = MidiEditorContext::NoteAttribute::Duration;

    void updateTimeLabels();
    void createTimeLabels();
    void updateCursorLabels();
    std::vector<Label*> timeLabels;

    Label* cursorTimeLabel = nullptr;
    Label* cursorPitchLabel = nullptr;
    Label* loopLabel = nullptr;
};