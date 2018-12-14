#pragma once

#include "ui.hpp"
#include <functional>

struct SqMenuItem : rack::MenuItem {
    void onAction(rack::EventAction &e) override {
      //  const bool econ = !theWidget->isEconomy();
      //  theWidget->setEconomy(econ);
     // printf("what do I do for SqMenuItem::action?\n"); fflush(stdout);
        _onActionFn();
	}

	void step() override {
        rightText = CHECKMARK(_isCheckedFn());
       // printf("what do I do for SqMenuItem::step?\n"); fflush(stdout);
    }

    SqMenuItem(std::function<bool()> isCheckedFn,
        std::function<void()> clickFn) :
            _isCheckedFn(isCheckedFn),
            _onActionFn(clickFn) {
    }

private:
    std::function<bool()> _isCheckedFn;
    std::function<void()> _onActionFn;
};