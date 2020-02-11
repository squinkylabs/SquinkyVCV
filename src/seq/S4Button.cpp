#include "InteropClipboard.h"
#include "MidiTrack4Options.h"
#include "MidiSelectionModel.h"
#include "S4Button.h"
#include "Seq4.h"
#include "../ctrl/SqUI.h"
#include "../ctrl/SqMenuItem.h"
#include "WidgetComposite.h"
#include "SqGfx.h"
#include "SqRemoteEditor.h"
#include "TimeUtils.h"
#include "UIPrefs.h"

#include <sstream>

/****************** context menu UI ********************/

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

class EditMenuItems : public  ::rack::ui::MenuItem
{
public:
    EditMenuItems(S4Button* opt) : button(opt)
    {
        text = "edit";
        rightText = RIGHT_ARROW;
    }
    ::rack::ui::Menu *createChildMenu() override
    {
        ::rack::ui::Menu* menu = new ::rack::ui::Menu();

        ::rack::ui::MenuItem* item =  new SqMenuItem([]{
            return false;
        }, [this]{
            button->doCut();
        });
        item->text = "cut";
        menu->addChild(item);
        ;
        item = new SqMenuItem([]{
            return false;
        }, [this]{
            button->doCopy();
        });
        item->text = "copy";
        menu->addChild(item);
        ;
        item =  new SqMenuItem([]{
            return false;
        }, [this]{
            button->doPaste();
        });
        item->text = "paste";
        menu->addChild(item);

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
    menu->addChild (new EditMenuItems(this));
    menu->addChild(new RepeatCountMenuItem(this));
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

    const int playStatus = seq4Comp->getPlayStatus(row);
    bool iAmPlaying = playStatus == (col + 1);
    if (iAmPlaying != isPlaying) {
        isPlaying = iAmPlaying;
        fw->dirty = true; 
    }

    const int nextSection = seq4Comp->getNextSection(row);
    bool isNext = (nextSection == (col + 1));
    if (iAmNext != isNext) {
        iAmNext = isNext;
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

void S4Button::doCut()
{
    doCopy();
    song->addTrack(row, col, nullptr);
}

void S4Button::doCopy()
{
    auto track = song->getTrack(row, col);
    if (track) {
        InteropClipboard::put(track, true);
    }
}

void S4Button::doPaste()
{
    MidiLocker l(song->lock);
    MidiTrackPtr destTrack = std::make_shared<MidiTrack>(song->lock, true); 
    destTrack->assertValid();

    // Make a fake selection that will say "select all".
    // It's a kluge that we need to provide an aution host.
   
    MidiSelectionModelPtr sel = std::make_shared<MidiSelectionModel>(nullptr, true);
    InteropClipboard::PasteData pasteData = InteropClipboard::get(0, destTrack, sel );
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

    if (!song) {
        WARN("no song to paste");
        return;
    }
    song->addTrack(row, col, destTrack);
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

inline S4Button::S4Button(
    const rack::math::Vec& size, 
    const rack::math::Vec& pos,
    int r, 
    int c, 
    MidiSong4Ptr s,
    std::shared_ptr<Seq4<WidgetComposite>> seq4) : row(r), col(c), song(s), seq4Comp(seq4)
{
    this->box.size = size;
    this->box.pos = pos;
    fw = new rack::widget::FramebufferWidget();
    this->addChild(fw);

    drawer = new S4ButtonDrawer(size, pos, this);
    fw->addChild(drawer);
}

inline void S4Button::setSelection(bool sel)
{
    if (_isSelected != sel) {
        _isSelected = sel;
        fw->dirty = true;
        if (_isSelected) {
            MidiTrackPtr tk = song->getTrack(row, col);
            if (tk) {
                SqRemoteEditor::client_announceData(tk);
            }
        }
    }
}

inline bool S4Button::handleKey(int key, int mods, int action)
{
    bool handled = false;
    
    if ((key == GLFW_KEY_V) && 
        (!(mods & RACK_MOD_CTRL)) &&
        (action == GLFW_PRESS)) {

        handled = true;
        doPaste();
    }
    return handled;
}

inline void S4Button::onSelectKey(const rack::event::SelectKey &e)
{
    bool handled = handleKey(e.key, e.mods, e.action);
    if (handled) {
        e.consume(this);
    } else {
        OpaqueWidget::onSelectKey(e);
    }
}

inline void S4Button::setClickHandler(callback h)
{
    clickHandler = h;
}

inline void S4Button::onDragEnter(const rack::event::DragEnter &e)
{
}

inline void S4Button::onDragLeave(const rack::event::DragLeave &e) 
{
    isDragging = false;
}


/***************************** S4ButtonGrid ***********************************/

using Comp = Seq4<WidgetComposite>;
void S4ButtonGrid::init(rack::app::ModuleWidget* parent, rack::engine::Module* module,
    MidiSong4Ptr song, std::shared_ptr<Seq4<WidgetComposite>> _seq4Comp)
{
    seq4Comp = _seq4Comp;
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
                song,
                seq4Comp);
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

void S4ButtonGrid::onClick(bool isCtrl, int row, int col)
{
    // first the selection logic
   for (int r = 0; r < MidiSong4::numTracks; ++r) {
        for (int c = 0; c < MidiSong4::numTracks; ++c) {
            auto button = getButton(r, c);
            assert(button);
            button->setSelection(r==row && c==col);
        }
    }
    // then the select next second
    // remember, section is 1..4
    seq4Comp->setNextSection(row, col + 1);     
}

std::function<void(bool isCtrlKey)> S4ButtonGrid::makeButtonHandler(int row, int col)
{
    return [this, row, col](bool isCtrl) {
        this->onClick(isCtrl, row, col);
    };
}




/********************** S4ButtonDrawer ****************/

/**
 * A special purpose button for the 4x4 seq module.
 * Has simple click handling, but lots of dedicated drawing ability
 */
void S4ButtonDrawer::draw(const DrawArgs &args)
{
    auto ctx = args.vg;
    paintButtonFace(ctx);
    paintButtonBorder(ctx);
    paintButtonText(ctx);
}

void S4ButtonDrawer::paintButtonFace(NVGcontext *ctx)
{
    auto color = button->isPlaying ? UIPrefs::X4_BUTTON_FACE_PLAYING : UIPrefs::X4_BUTTON_FACE_NORM;
    SqGfx::filledRect(
        ctx,
        color,
        this->box.pos.x, box.pos.y, box.size.x, box.size.y); 
}

void S4ButtonDrawer::paintButtonBorder(NVGcontext *ctx)
{
    NVGcolor color;
    float width = 0;
    bool draw = false;

    if (button->isSelected() && !button->iAmNext) {
        color = UIPrefs::X4_SELECTED_BORDER;
        width = 3;      // TODO: move to prefs
        draw = true; 
    } else if (!button->isSelected() && button->iAmNext) {
        color = UIPrefs::X4_NEXT_PLAY_BORDER;
        width = 2;      // TODO: move to prefs
        draw = true; 
    } else if (button->isSelected() && button->iAmNext) {
        //color = UIPrefs::X4_MIXED_BORDER;
       // width = 2;      // TODO: move to prefs
       // draw = true; 
        SqGfx::hBorder(
            ctx,
            2,
            UIPrefs::X4_NEXT_PLAY_BORDER,
            this->box.pos.x, box.pos.y, box.size.x, box.size.y);
        SqGfx::vBorder(
            ctx,
            4,
            UIPrefs::X4_SELECTED_BORDER,
            this->box.pos.x, box.pos.y, box.size.x, box.size.y);

       draw = false;
    }

    if (draw) {
        SqGfx::border(
            ctx, 
            width,
            color,
            this->box.pos.x, box.pos.y, box.size.x, box.size.y);
    }
}

void S4ButtonDrawer::paintButtonText(NVGcontext *ctx)
{
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


