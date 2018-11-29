#pragma once

#include <functional>

#include "IMWidgets.hpp"

// TODO: are these still used?
using CHBStatusCallback = std::function<bool()>;
using CHBActionCAllback =  std::function<void()>;

class IPanelHost
{
public:
    virtual void setExpanded(bool) =0;
    virtual bool isExpanded()=0;
};

/**
 * Bundles up all the handling of expanding and contracting
 * the panel. Relies on IPanelHost to talk back to the 
 * main widget. 
 */
class CHBPanelManager
{
public:
    CHBPanelManager(IPanelHost* );
    MenuItem*  createMenuItem();
    void makePanel(ModuleWidget* widget);
    void addMenuItems(Menu*);
    void poll();

private:
  //  bool expanded= false;
    DynamicSVGPanel* panel;
    int expWidth = 60;
    IPanelHost* const panelHost;
    ModuleWidget* widget = nullptr;

    void setPanelSize();
};

 CHBPanelManager::CHBPanelManager(IPanelHost* host) : panelHost(host)
 {

 }

inline  void CHBPanelManager::addMenuItems(Menu* menu)
{
    printf("addMenuItems\n"); fflush(stdout);
    menu->addChild(createMenuItem());
}

inline void CHBPanelManager::setPanelSize()
{
     widget->box.size = panel->box.size;
    const int expansionWidth = panelHost->isExpanded() ? 0 : -expWidth;
    widget->box.size.x += expansionWidth;
}

inline void CHBPanelManager::poll()
{
    setPanelSize();     // TODO: only do on change?
}

inline void CHBPanelManager::makePanel(ModuleWidget* wdg)
{
    widget = wdg;
    panel = new DynamicSVGPanel();

    panel->box.size = Vec(16 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    panel->box.size.x += expWidth;
 
	panel->expWidth = &expWidth;

    // I think this dynamically adds the real size. can get rid of the other stuff.
    panel->addPanel(SVG::load(assetPlugin(plugin, "res/cheby-wide-ghost.svg")));


    widget->box.size = panel->box.size;
    // printf("widget box a = %f exp=%f\n", widget->box.size.x, expWidth);
	
    // TODO: use setPanelSize
    const int expansionWidth = panelHost->isExpanded() ? 0 : -expWidth;
    widget->box.size.x += expansionWidth;
    printf("in ctor, exp = %d, exw = %d\n", panelHost->isExpanded(), expansionWidth);

    widget->addChild(panel);
}


/**************************************************************************
 **
 ** CHBPanelItem
 ** let's abstract this!
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

inline MenuItem*  CHBPanelManager::createMenuItem()
{
    auto statusCB = [this]() {
        return panelHost->isExpanded();
    };
    auto actionCB = [this]() {
       printf("panel setting ex from 133. will do something\n"); fflush(stdout);
       bool b = !panelHost->isExpanded();
       panelHost->setExpanded(b);
       setPanelSize();

    };
    return new CHBPanelItem(statusCB, actionCB);
}