#pragma once

#include <functional>

#include "IMWidgets.hpp"

using CHBStatusCallback = std::function<bool()>;
using CHBActionCAllback =  std::function<void()>;

class CHBPanelManager
{
public:
    MenuItem*  createMenuItem(CHBActionCAllback);
    bool isExpanded() const;
    void toggleExpanded();
    void makePanel(ModuleWidget* widget);

private:
    bool expanded= false;
    DynamicSVGPanel* panel;
    int expWidth = 60;
};

inline bool CHBPanelManager::isExpanded() const
{
    return expanded;
}

inline void CHBPanelManager::toggleExpanded()
{
    expanded = !expanded;
}

inline void CHBPanelManager::makePanel(ModuleWidget* widget)
{
    panel = new DynamicSVGPanel();

    panel->box.size = Vec(16 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    panel->box.size.x += expWidth;
 
	panel->expWidth = &expWidth;

    // I think this dynamically adds the real size. can get rid of the other stuff.
    panel->addPanel(SVG::load(assetPlugin(plugin, "res/chb_panel.svg")));


    widget->box.size = panel->box.size;
    // printf("widget box a = %f exp=%f\n", widget->box.size.x, expWidth);
	widget->box.size.x = widget->box.size.x - (1 - expanded) * expWidth;
    // printf("widget box witth = %f\n", widget->box.size.x);
    // fflush(stdout);
    widget->addChild(panel);
}


/**************************************************************************
 **
 **
 **
 ************************************************************************/

struct CHBPanelItem : MenuItem {

    CHBPanelItem(CHBStatusCallback, CHBActionCAllback);
    void onAction(EventAction &e) override {
         printf("ON ACTION\n");
        fflush(stdout);
        actionCallback();
	}

	void step() override {
        const bool b = statusCallback();
        rightText = CHECKMARK(b);
    }

    CHBStatusCallback statusCallback;
    CHBActionCAllback actionCallback;
};

inline CHBPanelItem::CHBPanelItem(CHBStatusCallback s, CHBActionCAllback a) :
    statusCallback(s),
    actionCallback(a)
{
    text = "Expanded Panel";
}

inline MenuItem*  CHBPanelManager::createMenuItem(CHBActionCAllback a)
{
    auto statusCB = [this]() {
        return isExpanded();
    };
    auto actionCB = [this, a]() {
        toggleExpanded();
        a();
    };
    return new CHBPanelItem(statusCB, actionCB);
}