#pragma once

#include <functional>

using CHBStatusCallback = std::function<bool()>;
using CHBActionCAllback =  std::function<void()>;

class CHBPanelManager
{
public:
    MenuItem*  createMenuItem(CHBActionCAllback);
    bool isExpanded() const;
    void toggleExpanded();

private:
    bool expanded= false;
};

inline bool CHBPanelManager::isExpanded() const
{
    return expanded;
}

inline void CHBPanelManager::toggleExpanded()
{
    expanded = !expanded;
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