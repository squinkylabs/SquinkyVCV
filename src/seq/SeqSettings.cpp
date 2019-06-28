//#include "SqMath.h"

#include "SeqSettings.h"
#include "../ctrl/SqMenuItem.h"



class GridMenuItem : public  rack::ui::MenuItem
{
public:
    GridMenuItem(SeqSettings* stt) : settings(stt)
    {
        text = "Grid settings";
    }

    SeqSettings* const settings;

    rack::ui::Menu *createChildMenu() override
    {
        rack::ui::Menu* menu = new rack::ui::Menu();

        auto label = rack::construct<rack::ui::MenuLabel>(
            &rack::ui::MenuLabel::text,
            "Grids             "); 
        menu->addChild(label);

        std::function<bool()> isCheckedFn = []() { return false; };

        rack::ui::MenuItem* item = new SqMenuItem(isCheckedFn, [this]() {
            settings->quarterNotesInGrid = 1;
        });
        item->text = "Quarter notes";
        menu->addChild(item);

        item = new SqMenuItem(isCheckedFn, [this]() {
            settings->quarterNotesInGrid = .5;
        });
        item->text = "Eighth notes";
        menu->addChild(item);

        item = new SqMenuItem(isCheckedFn, [this]() {
            settings->quarterNotesInGrid = .25;
        });
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
}

 float SeqSettings::getQuarterNotesInGrid()
 {
     return quarterNotesInGrid;
 }