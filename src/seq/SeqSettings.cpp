//#include "SqMath.h"

#include "SeqSettings.h"
#include "../ctrl/SqMenuItem.h"



class GridMenuItem : public  rack::ui::MenuItem
{
public:
    GridMenuItem()
    {
        text = "Grid settings";
    }

    rack::ui::Menu *createChildMenu() override
    {
        rack::ui::Menu* menu = new rack::ui::Menu();
        menu->addChild(rack::construct<rack::ui::MenuLabel>(
            &rack::ui::MenuLabel::text,
            "Grids"));

        std::function<bool()> isCheckedFn = []() { return false; };

        rack::ui::MenuItem* item = new SqMenuItem(isCheckedFn, []() {
                printf("do quarter grid\n"); fflush(stdout);
        });
        item->text = "Quarter Notes";
        menu->addChild(item);


        item = new SqMenuItem(isCheckedFn, []() {
                printf("do eigth grid\n"); fflush(stdout);
        });
        item->text = "Eighth Notes";
        menu->addChild(item);
        return menu;
    }
};


SeqSettings::SeqSettings(rack::engine::Module* mod) : module(mod)
{

}

void SeqSettings::invokeUI(rack::widget::Widget* parent)
{
    printf("creating menu\n"); fflush(stdout);
    rack::ui::Menu* menu = rack::createMenu();

    //menu->addChild(new rack::ui::MenuLabel);
    menu->addChild(rack::construct<rack::ui::MenuLabel>(&rack::ui::MenuLabel::text, "Seq++ Options"));
   // rack::ui::MenuItem* item = new rack::ui::MenuItem();
  //  item->text = "item text";
  //  item->rightText = "right";
    menu->addChild(new GridMenuItem());
}

 float SeqSettings::getSecondsInGrid()
 {
     // TODO: not seconds... quarter notes?
     return .25;
 }