//#include "SqMath.h"

#include "SeqSettings.h"
#include "../ctrl/SqMenuItem.h"
#include "TimeUtils.h"


class GridItem
{
public:
    GridItem() = delete;
    static rack::ui::MenuItem* make(SeqSettings::Grids grid, SeqSettings* stt)
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


/**
 * GridMenuItem is the whole grid selection sub-menu
 */
class GridMenuItem : public  rack::ui::MenuItem
{
public:
    GridMenuItem(SeqSettings* stt) : settings(stt)
    {
        text = "Grid settings";
        rightText = RIGHT_ARROW;
    }

    SeqSettings* const settings;

    rack::ui::Menu *createChildMenu() override
    {
        rack::ui::Menu* menu = new rack::ui::Menu();

        auto label = rack::construct<rack::ui::MenuLabel>(
            &rack::ui::MenuLabel::text,
            "Grids");      // need to do this to size correctly. probably doing something wrong.
        menu->addChild(label);

        rack::ui::MenuItem* item = GridItem::make(SeqSettings::Grids::quarter, settings);
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

SeqSettings::SeqSettings(rack::engine::Module* mod) : module(mod)
{
}

void SeqSettings::invokeUI(rack::widget::Widget* parent)
{
    rack::ui::Menu* menu = rack::createMenu();
    menu->addChild(rack::construct<rack::ui::MenuLabel>(&rack::ui::MenuLabel::text, "Seq++ Options"));
    menu->addChild(new GridMenuItem(this));
    menu->addChild(makeSnapItem());
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

    rack::ui::MenuItem* item = new SqMenuItem(isCheckedFn, clickFn);
    item->text = "Snap to grid";
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

float SeqSettings::getQuarterNotesInGrid()
{
    return grid2Time(curGrid);
}

 bool SeqSettings::snapToGrid()
 {
     return snapEnabled;
 }

 float SeqSettings::quantize(float time, bool allowZero)
 {
     auto quantized = time;
     if (snapToGrid()) {
        quantized = TimeUtils::quantize(time, getQuarterNotesInGrid(), allowZero);
     }
     return quantized;
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
