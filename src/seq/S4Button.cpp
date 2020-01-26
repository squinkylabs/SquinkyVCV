
#include "S4Button.h"
#include "seq4.h"
#include "../ctrl/SqUI.h"
#include "WidgetComposite.h"

 
MidiTrackPtr S4Button::getTrack() const
{
    return song->getTrack(row, col);
}

void S4Button::invokeContextMenu()
{
    assert(false);
}

void S4Button::step()
{
    ::rack::OpaqueWidget::step();

    auto track = getTrack();

    std::string newLen;
    float lengthTime = 0;
    int newNumNotes = 0;
    if (track) {
        lengthTime = track->getLength();
        newLen = TimeUtils::length2str(lengthTime);
        newNumNotes = track->size() - 1;
    } 
    if (newLen != contentLength) {
        // DEBUG("updating length %.2f, %s", length, newLen.c_str());
        contentLength = newLen;
        fw->dirty = true;
    }

    if (numNotes != newNumNotes) {
        numNotes = newNumNotes;
        fw->dirty = true;
    }
}

void S4Button::onDragHover(const rack::event::DragHover &e)
{
    sq::consumeEvent(&e, this);
}

void S4Button::onButton(const rack::event::Button &e)
{
    if ((e.button == GLFW_MOUSE_BUTTON_LEFT) && (e.action == GLFW_PRESS)) {
        // Do we need to consume this key to get dragLeave?
        isDragging = true;
        sq::consumeEvent(&e, this);
        return;
    }

    // release main button triggers click action
    if ((e.button == GLFW_MOUSE_BUTTON_LEFT) && (e.action == GLFW_RELEASE)) {
        // Command on mac.
        const bool ctrlKey = (e.mods & RACK_MOD_CTRL);

        if (!isDragging) {
            return;
        }

        // OK, process it
        sq::consumeEvent(&e, this);

        if (clickHandler) {
            clickHandler(ctrlKey);
        }
        return;
    }

    // alternate click brings up context menu
    if ((e.button == GLFW_MOUSE_BUTTON_RIGHT) && (e.action == GLFW_PRESS)) {
        invokeContextMenu();
        return;
    }

}

void S4Button::doPaste()
{
    auto clipData = SqClipboard::getTrackData();
    if (!clipData) {
        WARN("no clip data");
        return;
    }

    MidiTrackPtr track = clipData->track;
    if (!track) {
        WARN("no track on clip");
        return;
    }

    if (!song) {
        WARN("no song to paste");
        return;
    }
    song->addTrack(row, col, track);
    WARN("past length = %.2f", track->getLength());
    auto fnote = track->getFirstNote();
    if (fnote) {
        WARN("first note at time t %.2f", fnote->startTime);
    } else {
        WARN("No first note");
    }
}


/*****************************  ***********************************/

using Comp = Seq4<WidgetComposite>;
void S4ButtonGrid::init(rack::app::ModuleWidget* parent, rack::engine::Module* module, MidiSong4Ptr song)
{
  //  const float buttonSize = 50;
 //   const float buttonMargin = 10;
    const float jacksX = 380;
    for (int row = 0; row < MidiSong4::numTracks; ++row) {
        const float y = 70 + row * (buttonSize + buttonMargin);
        for (int col = 0; col < MidiSong4::numTracks; ++col) {
            const float x = 130 + col * (buttonSize + buttonMargin);
            S4Button* b = new S4Button(
                rack::math::Vec(buttonSize, buttonSize), 
                rack::math::Vec(x, y),
                row,
                col,
                song);
            parent->addChild(b);
            b->setClickHandler(makeButtonHandler(row, col));
            buttons[row][col] = b;
        }

        const float jacksY = y + 8;
        const float jacksDy = 28;
        
        parent->addOutput(rack::createOutputCentered<rack::componentlibrary::PJ301MPort>(
            rack::math::Vec(jacksX, jacksY),
            module,
            Comp::CV0_OUTPUT + row));
        parent->addOutput(rack::createOutputCentered<rack::componentlibrary::PJ301MPort>(
            rack::math::Vec(jacksX, jacksY + jacksDy),
            module,
            Comp::GATE0_OUTPUT + row));
    }
}

