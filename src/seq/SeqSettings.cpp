//#include "SqMath.h"

#include "SeqSettings.h"
#include "../ctrl/SqMenuItem.h"
#include "../SequencerModule.h"
#include "TimeUtils.h"

class GridItem
{
public:
    GridItem() = delete;
    static ::rack::ui::MenuItem* make(SeqSettings::Grids grid, SeqSettings* stt)
    {
        std::function<bool()> isCheckedFn = [stt, grid]() {
            return stt->curGrid == grid;
        };

        std::function<void()> clickFn = [stt, grid]() {
            stt->curGrid = grid;
        };

        return new SqMenuItem(isCheckedFn, clickFn);
    }
};

class ArticItem
{
public:
    ArticItem() = delete;
    static ::rack::ui::MenuItem* make(SeqSettings::Artics artic, SeqSettings* stt)
    {
        std::function<bool()> isCheckedFn = [stt, artic]() {
            return stt->curArtic == artic;
        };

        std::function<void()> clickFn = [stt, artic]() {
            stt->curArtic = artic;
        };

        return new SqMenuItem(isCheckedFn, clickFn);
    }
};


/**
* GridMenuItem is the whole grid selection sub-menu
*/
class GridMenuItem : public  ::rack::ui::MenuItem
{
public:
    GridMenuItem(SeqSettings* stt) : settings(stt)
    {
        text = "Grid settings";
        rightText = RIGHT_ARROW;
    }

    SeqSettings* const settings;

    ::rack::ui::Menu *createChildMenu() override
    {
        ::rack::ui::Menu* menu = new ::rack::ui::Menu();

        auto label = ::rack::construct<::rack::ui::MenuLabel>(
            &rack::ui::MenuLabel::text,
            "Grids");      // need to do this to size correctly. probably doing something wrong.
        menu->addChild(label);

        ::rack::ui::MenuItem* item = GridItem::make(SeqSettings::Grids::quarter, settings);
        item->text = "Quarter notes";
        menu->addChild(item);

        item = GridItem::make(SeqSettings::Grids::eighth, settings);
        item->text = "Eighth notes";
        menu->addChild(item);

        item = GridItem::make(SeqSettings::Grids::sixteenth, settings);
        item->text = "Sixteenth notes";
        menu->addChild(item);

        return menu;
    }
};

/**
*/
class ArticulationMenuItem : public  ::rack::ui::MenuItem
{
public:
    ArticulationMenuItem(SeqSettings* stt) : settings(stt)
    {
        text = "Articulation";
        rightText = RIGHT_ARROW;
    }

    ::rack::ui::Menu *createChildMenu() override
    {
        ::rack::ui::Menu* menu = new ::rack::ui::Menu();

        auto label = ::rack::construct<::rack::ui::MenuLabel>(
            &rack::ui::MenuLabel::text,
            "Articulation");      // need to do this to size correctly. probably doing something wrong.
        menu->addChild(label);

        ::rack::ui::MenuItem* item = ArticItem::make(SeqSettings::Artics::tenPercent, settings);
        item->text = "10%";
        menu->addChild(item);

        item = ArticItem::make(SeqSettings::Artics::twentyPercent, settings);
        item->text = "20%";
        menu->addChild(item);

        item = ArticItem::make(SeqSettings::Artics::fiftyPercent, settings);
        item->text = "50%";
        menu->addChild(item);

        item = ArticItem::make(SeqSettings::Artics::eightyFivePercent, settings);
        item->text = "85%";
        menu->addChild(item);

        item = ArticItem::make(SeqSettings::Artics::oneHundredPercent, settings);
        item->text = "100%";
        menu->addChild(item);

        item = ArticItem::make(SeqSettings::Artics::legato, settings);
        item->text = "legato";
        menu->addChild(item);

        return menu;
    }
private:
    SeqSettings* const settings;
};

SeqSettings::SeqSettings(SequencerModule* mod) : module(mod)
{
    ++_mdb;
}

SeqSettings::~SeqSettings()
{
    --_mdb;
}

void SeqSettings::invokeUI (::rack::widget::Widget* parent)
{
    ::rack::ui::Menu* menu = ::rack::createMenu();
    menu->addChild (::rack::construct<::rack::ui::MenuLabel>(&rack::ui::MenuLabel::text, "Seq++ Options"));
    menu->addChild(new GridMenuItem(this));
    menu->addChild(makeSnapItem());
    menu->addChild(makeSnapDurationItem());
    menu->addChild(makeAuditionItem(module));
    menu->addChild(new ArticulationMenuItem(this));
    menu->addChild(makeLoopItem(module));
    // now the commands
    menu->addChild(new ::rack::ui::MenuLabel);
    menu->addChild(makeNoteCommand(module));
    menu->addChild(makeEndCommand(module));
}

rack::ui::MenuItem* SeqSettings::makeLoopItem(SequencerModule* module)
{
     std::function<bool()> isCheckedFn = [module]() {
        bool ret = false;
        MidiSequencerPtr seq = module->getSeq();
        if (seq) {
            ret = seq->editor->isLooped();
        }
        return ret;
    };

     std::function<void()> clickFn = [module]() {
        MidiSequencerPtr seq = module->getSeq();
        if (seq) {
            seq->editor->loop();
        }
    };

    auto item = new SqMenuItem(isCheckedFn, clickFn);
    item->text = "Loop subrange";
    return item;
}
rack::ui::MenuItem* SeqSettings::makeNoteCommand(SequencerModule* module)
{
    const bool isNote = bool(module->getSeq()->editor->getNoteUnderCursor());
    std::function<bool()> isCheckedFn = []() {
        return false;
    };

    std::function<void()> clickFn = [module, isNote]() {
        MidiSequencerPtr seq = module->getSeq();
        MidiEditorPtr editor =seq->editor;
        if (isNote) {
            editor->deleteNote();
        } else {
            editor->insertDefaultNote(false, false);
        }
    };

    auto item = new SqMenuItem(isCheckedFn, clickFn);
    item->text = isNote ? "Delete note" : "Insert note";
    return item;
}

rack::ui::MenuItem* SeqSettings::makeEndCommand(SequencerModule* module)
{
    std::function<bool()> isCheckedFn = []() {
        return false;
    };

    std::function<void()> clickFn = [module]() {
        MidiEditorPtr editor = module->getSeq()->editor;
        editor->changeTrackLength();
    };
    auto item = new SqMenuItem(isCheckedFn, clickFn);
    item->text = "Set end point";
    return item;
}

rack::ui::MenuItem* SeqSettings::makeAuditionItem(SequencerModule* module)
{
    int id = module->getAuditionParamId();
    ::rack::ui::MenuItem* item = new SqMenuItem_BooleanParam2(module, id);
    item->text = "Audition";
    return item;
}

rack::ui::MenuItem* SeqSettings::makeSnapItem()
{
    bool& snap = this->snapEnabled;
    std::function<bool()> isCheckedFn = [snap]() {
        return snap;
    };

    std::function<void()> clickFn = [&snap]() {
        snap = !snap;
    };

    ::rack::ui::MenuItem* item = new SqMenuItem(isCheckedFn, clickFn);
    item->text = "Snap to grid";
    return item;
}

rack::ui::MenuItem* SeqSettings::makeSnapDurationItem()
{
    bool& snap = this->snapDurationEnabled;
    std::function<bool()> isCheckedFn = [snap]() {
        return snap;
    };

    std::function<void()> clickFn = [&snap]() {
        snap = !snap;
    };

    ::rack::ui::MenuItem* item = new SqMenuItem(isCheckedFn, clickFn);
    item->text = "Snap duration to grid";
    return item;
}

float SeqSettings::grid2Time(Grids g)
{
    float time = 1;         // default to quarter note
    switch (g) {
    case Grids::quarter:
        time = 1;
        break;
    case Grids::eighth:
        time = .5f;
        break;
    case Grids::sixteenth:
        time = .25f;
        break;
    default:
        assert(false);
    }
    return time;
}

float SeqSettings::artic2Number(Artics a)
{
    float artic = .85f; 
    switch (a) {
    case Artics::eightyFivePercent:
        artic = .85f;
        break;
    case Artics::legato:
        artic = 1.01f;
        break;
    case Artics::tenPercent:
        artic = .1f;
        break;
    case Artics::twentyPercent:
        artic = .2f;
        break;
    case Artics::fiftyPercent:
        artic = .5f;
        break;
    case Artics::oneHundredPercent:
        artic = 1.0f;
        break;
    default:
        assert(false);
    }
    return artic;
}


float SeqSettings::getQuarterNotesInGrid()
{
    return grid2Time(curGrid);
}

bool SeqSettings::snapToGrid()
{
    return snapEnabled;
}

bool SeqSettings::snapDurationToGrid()
{
    return snapDurationEnabled;
}

float SeqSettings::quantize(float time, bool allowZero)
{
    auto quantized = time;
    if (snapToGrid()) {
        quantized = TimeUtils::quantize(time, getQuarterNotesInGrid(), allowZero);
    }
    return quantized;
}

float SeqSettings::quantizeAlways(float time, bool allowZero)
{
    return (float) TimeUtils::quantize(time, getQuarterNotesInGrid(), allowZero);
}

std::string SeqSettings::getGridString() const
{
    std::string ret;
    switch(curGrid) {
    case Grids::quarter:
        ret = "quarter";
        break;
    case Grids::eighth:
        ret = "eighth";
        break;
    case Grids::sixteenth:
        ret = "sixteenth";
        break;
    default:
        assert(false);
    }
    return ret;
}

std::string SeqSettings::getArticString() const
{
    std::string ret;
    switch(curArtic) {
    case Artics::eightyFivePercent:
        ret = "85%";
        break;
    case Artics::legato:
        ret = "legato";
        break;
    case Artics::oneHundredPercent:
        ret = "100%";
        break;
    case Artics::tenPercent:
        ret = "10%";
        break;
    case Artics::twentyPercent:
        ret = "20%";
        break;
    case Artics::fiftyPercent:
        ret = "50%";
        break;
    default:
        assert(false);
    }
    return ret;
}

SeqSettings::Grids SeqSettings::gridFromString(const std::string& s)
{
    Grids ret = Grids::sixteenth;
    if (s == "sixteenth") {
        ret = Grids::sixteenth;
    } else if (s == "eighth") {
        ret = Grids::eighth;
    } else if ( s == "quarter") {
        ret = Grids::quarter;
    } else {
        assert(false);
    }
    return ret;
}

SeqSettings::Artics SeqSettings::articFromString(const std::string& s)
{
    Artics ret = Artics::eightyFivePercent;
    if (s == "85%") {
        ret = Artics::eightyFivePercent;
    } else if (s == "10%") {
        ret = Artics::tenPercent;
    } else if ( s == "100%") {
        ret = Artics::oneHundredPercent;
    } else if (s == "legato") {
        ret = Artics::legato;
    } else if (s == "20%") {
        ret = Artics::twentyPercent;
    } else if (s == "50%") {
        ret = Artics::fiftyPercent;
    } else {
        assert(false);
    }
    return ret;
}

float SeqSettings::articulation() 
{
    return artic2Number(curArtic);
}

std::string SeqSettings::getMidiFilePath() 
{
    return midiFilePath;
}

void SeqSettings::setMidiFilePath(const std::string& s)
{
    midiFilePath = s;
}
