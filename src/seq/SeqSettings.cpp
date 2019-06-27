
#include "SeqSettings.h"



class GridMenuItem : public  rack::ui::MenuItem
{
public:
    GridMenuItem()
    {
        text = "Grid settings";
    }
    #if 1
    rack::ui::Menu *createChildMenu() override
    {
        rack::ui::Menu* menu = new rack::ui::Menu();
        menu->addChild(rack::construct<rack::ui::MenuLabel>(
            &rack::ui::MenuLabel::text,
            "Grids"));
        rack::ui::MenuItem* item = new rack::ui::MenuItem();
        item->text = "Quarter Notes";
        menu->addChild(item);

         item = new rack::ui::MenuItem();
        item->text = "Eighth Notes";
        menu->addChild(item);
        return menu;
    }
    #endif
};

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