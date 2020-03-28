#include "../Squinky.hpp"
#include "S4Button.h"
#include "S4ButtonGrid.h"
#include "../ctrl/SqMenuItem.h"
#include "../ctrl/SqUI.h"
#include "InteropClipboard.h"
#include "MakeEmptyTrackCommand4.h"
#include "MidiSelectionModel.h"
#include "MidiTrack4Options.h"
#include "MidiSong4.h"
#include "Seq4.h"
#include "SqGfx.h"
#include "SqRemoteEditor.h"
#include "TimeUtils.h"
#include "UIPrefs.h"
#include "UndoRedoStack.h"
#include "WidgetComposite.h"

#include <sstream>

#ifdef _SEQ4

/****************** context menu UI ********************/

class RepeatItem : public ::rack::ui::MenuItem {
public:
    RepeatItem() = delete;
    static ::rack::ui::MenuItem* make(S4Button* button, int value) {
        std::function<bool()> isCheckedFn = [button, value]() {
            return button->getRepeatCountForUI() == value;
        };

        std::function<void()> clickFn = [button, value]() {
            button->setRepeatCountForUI(value);
        };

        return new SqMenuItem(isCheckedFn, clickFn);
    }
};

class RepeatCountMenuItem : public ::rack::ui::MenuItem {
public:
    RepeatCountMenuItem(S4Button* opt) : button(opt) {
        text = "repeat count";
        rightText = RIGHT_ARROW;
    }

    ::rack::ui::Menu* createChildMenu() override {
        ::rack::ui::Menu* menu = new ::rack::ui::Menu();

        auto label = ::rack::construct<::rack::ui::MenuLabel>(
            &rack::ui::MenuLabel::text,
            "Repeat Count");
        menu->addChild(label);
        ::rack::ui::MenuItem* item = RepeatItem::make(button, 0);
        item->text = "Forever";
        menu->addChild(item);

        for (int i = 1; i <= 16; ++i) {
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

class EditMenuItems : public ::rack::ui::MenuItem {
public:
    EditMenuItems(S4Button* opt) : button(opt) {
        text = "edit";
        rightText = RIGHT_ARROW;
    }
    ::rack::ui::Menu* createChildMenu() override {
        ::rack::ui::Menu* menu = new ::rack::ui::Menu();

        ::rack::ui::MenuItem* item = new SqMenuItemAccel("X", [this] {
            button->doCut();
        });
        item->text = "Cut";
        menu->addChild(item);

        item = new SqMenuItemAccel("C", [this] {
            button->doCopy();
        });
        item->text = "Copy";
        menu->addChild(item);

        item = new SqMenuItemAccel("V", [this] {
            button->doPaste();
        });
        item->text = "Paste";
        menu->addChild(item);

        return menu;
    }

private:
    S4Button* const button;
};

void S4Button::otherItems(::rack::ui::Menu* menu) {
    ::rack::ui::MenuLabel* spacerLabel = new ::rack::ui::MenuLabel();
    menu->addChild(spacerLabel);

    ::rack::ui::MenuItem* item = new SqMenuItemAccel("Click", [this]() {
        clickHandler(false);
    });
    item->text = "Set next section";
    menu->addChild(item);

    item = new SqMenuItemAccel("Ctrl+Click", [this]() {
        clickHandler(true);
    });
    item->text = "Set next clip";
    menu->addChild(item);

    item = new SqMenuItemAccel("", [this]() {
        doEditClip();
    });

    item->text = "Edit clip";
    menu->addChild(item);
};

//*********************** S4Button ************************/

MidiTrackPtr S4Button::getTrack() const {
    return seq->song->getTrack(row, col);
}

MidiTrack4OptionsPtr S4Button::getOptions() const {
    return seq->song->getOptions(row, col);
}

void S4Button::invokeContextMenu() {
    ::rack::ui::Menu* menu = ::rack::createMenu();
    menu->addChild(::rack::construct<::rack::ui::MenuLabel>(&rack::ui::MenuLabel::text, "4X4 Pad Menu"));
    menu->addChild(new EditMenuItems(this));
    menu->addChild(new RepeatCountMenuItem(this));
    otherItems(menu);
}

void S4Button::step() {
    auto track = getTrack();

    std::string newLen;
    float lengthTime = 0;
    int newNumNotes = 0;
    int repetitionIndex = 1;
    int newRepeatCount = this->repeatCount;
    if (track) {
        lengthTime = track->getLength();
        newLen = TimeUtils::length2str(lengthTime);
        newNumNotes = track->size() - 1;

        // if no comp, make one up for the module browser
        repetitionIndex = seq4Comp ? seq4Comp->getTrackPlayer(row)->getCurrentRepetition() : 1;

        auto options = getOptions();
        if (options) {
            newRepeatCount = options->repeatCount;
        }
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

    if (repetitionIndex != repetitionNumber) {
        repetitionNumber = repetitionIndex;
        fw->dirty = true;
    }

    if (repeatCount != newRepeatCount) {
        repeatCount = newRepeatCount;
        fw->dirty = true;
    }

    const int playStatus = seq4Comp ? seq4Comp->getPlayStatus(row) : 1;
    bool iAmPlaying = playStatus == (col + 1);
    if (iAmPlaying != isPlaying) {
        isPlaying = iAmPlaying;
        fw->dirty = true;
    }

    const int nextSection = seq4Comp ? seq4Comp->getNextSectionRequest(row) : 0;
    bool isNext = (nextSection == (col + 1));
    if (iAmNext != isNext) {
        iAmNext = isNext;
        fw->dirty = true;
    }
    pollForParamChange();

    ::rack::app::ParamWidget::step();
}

void S4Button::pollForParamChange()
{
    if (!module) {
        return;
    }
   
    const bool paramIsOn = ::rack::appGet()->engine->getParam(module, selectParamId) > .5;
    if (paramIsOn != lastSelectParamState) {
        if (paramIsOn) {
            // treat this like a control click
            if (clickHandler) {
                clickHandler(true);
            }
        }
        lastSelectParamState = paramIsOn;
    }
}


void S4Button::doCut() {
    doCopy();
    Command4Ptr cmd = MakeEmptyTrackCommand4::createRemoveTrack(seq, row, col, TimeUtils::bar2time(2));
    seq->undo->execute4(seq, cmd);
    //seq->undo->
    //song->addTrack(row, col, nullptr);
}

void S4Button::doCopy() {
    auto track = seq->song->getTrack(row, col);
    if (track) {
        InteropClipboard::put(track, true);
    }
}

void S4Button::doPaste() {
    MidiLocker l(seq->song->lock);
    MidiTrackPtr destTrack = std::make_shared<MidiTrack>(seq->song->lock, true);
    destTrack->assertValid();

    // Make a fake selection that will say "select all".
    // It's a kluge that we need to provide an aution host.

    MidiSelectionModelPtr sel = std::make_shared<MidiSelectionModel>(nullptr, true);
    InteropClipboard::PasteData pasteData = InteropClipboard::get(0, destTrack, sel);
    // So far, dest track has just been a refernce track for delting notes we might paste
    // on top of.
    // now let's put all the data in there (re-use it)
    assert(destTrack->size() == 1);
    assert(pasteData.toRemove.empty());

    destTrack->setLength(pasteData.requiredTrackLength);
    for (auto n : pasteData.toAdd) {
        MidiEventPtr event = n;
        destTrack->insertEvent(n);
    }

    destTrack->assertValid();

    if (!seq->song) {
        WARN("no song to paste");
        return;
    }
    seq->song->addTrack(row, col, destTrack);
}

int S4Button::getRepeatCountForUI() {
    auto options = getOptions();
    if (options) {
        return options->repeatCount;
    } else {
        return 0;
    }
}

void S4Button::setRepeatCountForUI(int ct) {
    auto options = getOptions();
    if (options) {
        options->repeatCount = ct;
    } else {
        WARN("editing repeats when no data");
    }
}

S4Button::S4Button(
    const rack::math::Vec& size,
    const rack::math::Vec& pos,
    int r,
    int c,
    MidiSequencer4Ptr s,
    std::shared_ptr<Seq4<WidgetComposite>> seq4,
    ::rack::engine::Module* theModule) : row(r), col(c), seq(s), seq4Comp(seq4),
    module(theModule),
    selectParamId(Seq4<WidgetComposite>::PADSELECT0_PARAM + col + row * MidiSong4::numSectionsPerTrack)
{

    fw = new rack::widget::FramebufferWidget();
    this->addChild(fw);

    S4ButtonDrawer* drawer = new S4ButtonDrawer(size, this);
    // INFO("got back drawer = %p", drawer);
    // INFO("drawer1 size = %f, %f", drawer->box.size.x, drawer->box.size.y);
    fw->addChild(drawer);

    // INFO("add child fb for s4 button size = %f,%f", fw->box.size.x, fw->box.size.y);
    // INFO("drawer2 size = %f, %f", drawer->box.size.x, drawer->box.size.y);

    // INFO("\nctor of button, pos %.2f, %.2f", pos.x, pos.y);
    // INFO("ctor of button, siz %.2f, %.2f", size.x, size.y);
    // INFO(" button = %p\n", this);
    this->box.size = size;
    this->box.pos = pos;
}

void S4Button::doEditClip() {
    MidiTrackPtr tk = seq->song->getTrack(row, col);
    INFO("S4Button::doEditClip getting track from %d, %d", row, col);
    if (!tk) {
        // make a new track on click, if needed
        MidiLocker l(seq->song->lock);
        tk = MidiTrack::makeEmptyTrack(seq->song->lock);
        seq->song->addTrack(row, col, tk);
    }

    SqRemoteEditor::clientAnnounceData(tk);
}

void S4Button::setSelection(bool sel) {
    if (_isSelected != sel) {
        _isSelected = sel;
        fw->dirty = true;
    }
}

bool S4Button::handleKey(int key, int mods, int action) {
    bool handled = false;

    if (!(mods & RACK_MOD_CTRL) &&
        (action == GLFW_PRESS)) {
        switch (key) {
            case GLFW_KEY_X:
                doCut();
                handled = true;
                break;
            case GLFW_KEY_C:
                doCopy();
                handled = true;
                break;
            case GLFW_KEY_V:
                doPaste();
                handled = true;
                break;
        }
    }
    return handled;
}

void S4Button::onDragHover(const rack::event::DragHover& e) {
    sq::consumeEvent(&e, this);
}

#if 1
void S4Button::onButton(const rack::event::Button& e) {

    if ((e.button == GLFW_MOUSE_BUTTON_LEFT) && (e.action == GLFW_PRESS) && (e.mods & RACK_MOD_CTRL)) {
        mouseButtonIsControlKey = true;
        sq::consumeEvent(&e, this);
        return;
    }
    mouseButtonIsControlKey = false;

     // alternate click brings up context menu
    if ((e.button == GLFW_MOUSE_BUTTON_RIGHT) && (e.action == GLFW_PRESS)) {
        sq::consumeEvent(&e, this);
        invokeContextMenu();
        return;
    }


    ::rack::app::ParamWidget::onButton(e);
}
#endif

void S4Button::onDragStart(const rack::event::DragStart& e) {

    if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
         if (clickHandler) {
            clickHandler(mouseButtonIsControlKey);
        }
    }
    mouseButtonIsControlKey = false;
}


void S4Button::onSelectKey(const rack::event::SelectKey& e) {
    bool handled = handleKey(e.key, e.mods, e.action);
    if (handled) {
        e.consume(this);
    } else {
        ParamWidget::onSelectKey(e);
    }
}

void S4Button::setClickHandler(callback h) {
    clickHandler = h;
}

/********************** S4ButtonDrawer ****************/

S4ButtonDrawer::S4ButtonDrawer(const rack::math::Vec& size, S4Button* button) : button(button) {
    this->box.size = size;
}

/**
 * A special purpose button for the 4x4 seq module.
 * Has simple click handling, but lots of dedicated drawing ability
 */
void S4ButtonDrawer::draw(const ::rack::widget::Widget::DrawArgs& args) {
    auto ctx = args.vg;
    paintButtonFace(ctx);
    paintButtonBorder(ctx);
    paintButtonText(ctx);
}

void S4ButtonDrawer::paintButtonFace(NVGcontext* ctx) {

    NVGcolor color = UIPrefs::X4_BUTTON_FACE_NORM;

    if (button->isPlaying && (button->numNotes > 0)) {
        // playing, notes
        color = UIPrefs::X4_BUTTON_FACE_PLAYING;
    } else if (!button->isPlaying && button->isSelected()) {
        // not playing, selected
        color = UIPrefs::X4_BUTTON_FACE_SELECTED;
    } else if (button->isPlaying && (button->numNotes == 0)) {
        // playing, no notes
        color = UIPrefs::X4_BUTTON_FACE_NONOTES_PLAYING;
    } else if (!button->isPlaying && (button->numNotes > 0)) {
        // not playing, notes
        color = UIPrefs::X4_BUTTON_FACE_NORM;
  //  } else if (button->isSelected() && (button->numNotes <= 0)) {
 //       color = UIPrefs::X4_BUTTON_FACE_NONOTES_SELECTED
    } else {
        color = UIPrefs::X4_BUTTON_FACE_NONOTES;
    }

    // just for test.
   // color = UIPrefs::NOTE_COLOR;

    SqGfx::filledRect(
        ctx,
        color,
        box.pos.x, box.pos.y, box.size.x, box.size.y);
}

void S4ButtonDrawer::paintButtonBorder(NVGcontext* ctx) {
   
    float width = 2;

    if (button->iAmNext) {
        SqGfx::border(
            ctx,
            width,
            UIPrefs::X4_NEXT_PLAY_BORDER,
            box.pos.x, box.pos.y, box.size.x, box.size.y);
    }
}

void S4ButtonDrawer::paintButtonText(NVGcontext* ctx) {
    nvgTextAlign(ctx, NVG_ALIGN_CENTER);
    nvgBeginPath(ctx);
    nvgFontSize(ctx, 14.f);
    nvgFillColor(ctx, UIPrefs::TIME_LABEL_COLOR);
    nvgText(ctx,  S4ButtonGrid::buttonSize / 2, 15, button->contentLength.c_str(), nullptr);

    if (!button->contentLength.empty() && (button->repeatCount > 0)) {
        std::stringstream s;
        if ( button->isPlaying) {
            s << button->repetitionNumber;
            s << "/";
            s << button->repeatCount; 
        } else {
            s << button->repeatCount; 
        }
        nvgText(ctx, S4ButtonGrid::buttonSize / 2, 45, s.str().c_str(), nullptr);
    }
}

#endif