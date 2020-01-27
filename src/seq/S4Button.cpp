
#include "MidiTrack4Options.h"
#include "S4Button.h"
#include "seq4.h"
#include "../ctrl/SqUI.h"
#include "../ctrl/SqMenuItem.h"
#include "WidgetComposite.h"
#include "SqGfx.h"
#include "TimeUtils.h"
#include "UIPrefs.h"

#include <sstream>

/****************** sontext menu UI ********************/

class RepeatItem  : public  ::rack::ui::MenuItem
{
public:
    RepeatItem() = delete;
    static ::rack::ui::MenuItem* make(S4Button* button, int value)
    {
        std::function<bool()> isCheckedFn = [button, value]() {
            return button->getRepeatCountForUI() == value;
        };

        std::function<void()> clickFn = [button, value]() {
            button->setRepeatCountForUI(value);
        };

        return new SqMenuItem(isCheckedFn, clickFn);
    }
};

class RepeatCountMenuItem : public  ::rack::ui::MenuItem
{
public:
    RepeatCountMenuItem(S4Button* opt) : button(opt)
    {
        text = "repeat count";
        rightText = RIGHT_ARROW;
    }

    ::rack::ui::Menu *createChildMenu() override
    {
        ::rack::ui::Menu* menu = new ::rack::ui::Menu();

        auto label = ::rack::construct<::rack::ui::MenuLabel>(
            &rack::ui::MenuLabel::text,
            "Repeat Count");      
        menu->addChild(label);
#if 0 // we don't support this yet
        ::rack::ui::MenuItem* item = RepeatItem::make(button, 0);
        item->text = "Forever";
        menu->addChild(item);
#endif

        for (int i=1; i<=16; ++i) {
            ::rack::ui::MenuItem* item = RepeatItem::make(button, i);
            std::stringstream str;
            str << i;
            item->text = str.str();
            menu->addChild(item);
        }

        return menu;
    }
private:
    S4Button* const button; 
};

//*********************** S4Button ************************/

 
MidiTrackPtr S4Button::getTrack() const
{
    return song->getTrack(row, col);
}

MidiTrack4OptionsPtr S4Button::getOptions() const
{
    return song->getOptions(row, col);
}
void S4Button::invokeContextMenu()
{
    ::rack::ui::Menu* menu = ::rack::createMenu();
    menu->addChild (::rack::construct<::rack::ui::MenuLabel>(&rack::ui::MenuLabel::text, "Section Options"));

    menu->addChild(new RepeatCountMenuItem(this));

    //should we return the menu?
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
        sq::consumeEvent(&e, this);
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

int S4Button::getRepeatCountForUI()
{
    auto options = getOptions();
    if (options) {
        return options->repeatCount;
    } else {
        WARN("editing repeats when no data");
        return 0;
    }
}

void S4Button::setRepeatCountForUI(int ct)
{
    auto options = getOptions();
    if (options) {
        options->repeatCount = ct;;
    } else {
        WARN("editing repeats when no data");
    }
}
/***************************** S4ButtonGrid ***********************************/

using Comp = Seq4<WidgetComposite>;
void S4ButtonGrid::init(rack::app::ModuleWidget* parent, rack::engine::Module* module, MidiSong4Ptr song)
{
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




/********************** S4ButtonDrawer ****************/

/**
 * A special purpose button for the 4x4 seq module.
 * Has simple click handling, but lots of dedicated drawing ability
 */
void S4ButtonDrawer::draw(const DrawArgs &args)
{
    auto ctx = args.vg;
    if (button->isSelected()) {
          SqGfx::filledRect(
                args.vg,
                UIPrefs::X4_SELECTION_COLOR,
                this->box.pos.x, box.pos.y, box.size.x, box.size.y); 
    } else {
        SqGfx::filledRect(
                args.vg,
                UIPrefs::NOTE_COLOR,
                this->box.pos.x, box.pos.y, box.size.x, box.size.y); 
                //x, y, width, noteHeight);
    }

    nvgBeginPath(ctx);
    nvgFontSize(ctx, 14.f);
    nvgFillColor(ctx, UIPrefs::TIME_LABEL_COLOR);
    nvgText(ctx, 5, 15, button->contentLength.c_str(), nullptr);
    if (button->numNotes > 0) {
        std::stringstream s;
        s << button->numNotes;
        nvgText(ctx, 5, 30, s.str().c_str(), nullptr);
    }
}


