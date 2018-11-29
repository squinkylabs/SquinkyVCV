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
    MenuItem*  createMenuItem(CHBActionCAllback);
  //  bool isExpanded() const;
 //   void toggleExpanded();
    void makePanel(ModuleWidget* widget);
    void addMenuItems(Menu*);

private:
  //  bool expanded= false;
    DynamicSVGPanel* panel;
    int expWidth = 60;
    IPanelHost* const panelHost;
};

 CHBPanelManager::CHBPanelManager(IPanelHost* host) : panelHost(host)
 {

 }

inline  void CHBPanelManager::addMenuItems(Menu*)
{
    printf("addMenuItems nimp\n"); fflush(stdout);
}

#if 0
inline bool CHBPanelManager::isExpanded() const
{
    return expanded;
}


inline void CHBPanelManager::toggleExpanded()
{
    expanded = !expanded;
}
#endif

inline void CHBPanelManager::makePanel(ModuleWidget* widget)
{
    panel = new DynamicSVGPanel();

    panel->box.size = Vec(16 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    panel->box.size.x += expWidth;
 
	panel->expWidth = &expWidth;

    // I think this dynamically adds the real size. can get rid of the other stuff.
    panel->addPanel(SVG::load(assetPlugin(plugin, "res/cheby-wide-ghost.svg")));


    widget->box.size = panel->box.size;
    // printf("widget box a = %f exp=%f\n", widget->box.size.x, expWidth);
	
    
    const int expansionWidth = panelHost->isExpanded() ? expWidth : 0;
    widget->box.size.x += expansionWidth;

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

// TODO: we don't need action callback
inline MenuItem*  CHBPanelManager::createMenuItem(CHBActionCAllback a)
{
    auto statusCB = [this]() {
        return panelHost->isExpanded();
    };
    auto actionCB = [this, a]() {
       // toggleExpanded();
       // a();
       bool b = !panelHost->isExpanded();
       panelHost->setExpanded(b);

    };
    return new CHBPanelItem(statusCB, actionCB);
}