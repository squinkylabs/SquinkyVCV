

#include "../Squinky.hpp"
#ifdef __V1x
#include "widget/Widget.hpp"
#else
#include "widgets.hpp"
#include "util/math.hpp"
#endif

#include "MidiEditorContext.h"

class MidiSequencer;
using  MidiSequencerPtr = std::shared_ptr<MidiSequencer>;

struct AboveNoteGrid : OpaqueWidget
{
public:
    AboveNoteGrid(const Vec& pos, const Vec& size, MidiSequencerPtr seq);

    /**
     * Inject a new sequencer into this editor.
     */
    void setSequencer(MidiSequencerPtr seq);

#ifdef __V1x
    void draw(const DrawArgs &args) override;
#else
    void draw(NVGcontext *vg) override;
#endif
    void step() override;

private:
    bool firstTime = true;
    int curFirstBar = -1;      // number of measure at start of grid
    float curCursorTime = -1;
    float curCursorPitch = -1;
    MidiSequencerPtr sequencer;
    Label* editAttributeLabel = nullptr;
    MidiEditorContext::NoteAttribute curAttribute = MidiEditorContext::NoteAttribute::Duration;

    void updateTimeLabels();
    void createTimeLabels();
    void updateCursorLabels();
    std::vector<Label*> timeLabels;

    Label* cursorTimeLabel = nullptr;
    Label* cursorPitchLabel = nullptr;
};