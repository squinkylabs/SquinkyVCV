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

#include "engine.hpp"
struct  SqMenuItem_BooleanParam : rack::MenuItem {

  
    SqMenuItem_BooleanParam(rack::Module* module, int paramId) :
        paramId(paramId),
        module(module) {
    }
    void onAction(rack::EventAction &e) override {
        const float newValue = isOn() ? 0 : 1;
        engineSetParam(module, paramId, newValue);
    }

    void step() override {
        rightText = CHECKMARK(isOn());
    }

private:
    bool isOn()
    {
        return module->params[paramId].value > .5f;
    }
    const int paramId;
    rack::Module* const  module;
};