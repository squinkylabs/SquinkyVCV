#pragma once

#include <string>
#include <vector>

#include "SqMenuItem.h"

class SubMenuParamCtrl : public ::rack::MenuItem {
public:
    using RenderFunc = std::function<std::string(int)>;
    static SubMenuParamCtrl* create(Menu*, const std::string& label, const std::vector<std::string>& children,
                       Module*, int param, RenderFunc func);
    static SubMenuParamCtrl* create(Menu*, const std::string& label, const std::vector<std::string>& children,
                       Module*, int param);
    ::rack::ui::Menu* createChildMenu() override;

private:
    SubMenuParamCtrl(const std::vector<std::string>& children, Module*, int param, RenderFunc func);
    int getCurrentSetting();
    void setParam(int value);
    const std::vector<std::string> items;
    Module* const module;
    const int paramNumber;
    RenderFunc const func = nullptr;
};

inline SubMenuParamCtrl* SubMenuParamCtrl::create(
    Menu* menu,
    const std::string& label,
    const std::vector<std::string>& children,
    Module* module,
    int param,
    RenderFunc func) {

    SubMenuParamCtrl* temporaryThis = new SubMenuParamCtrl(children, module, param, func);
    temporaryThis->text = label;
    menu->addChild(temporaryThis);
    return temporaryThis;
}

inline SubMenuParamCtrl* SubMenuParamCtrl::create(
    Menu* menu,
    const std::string& label,
    const std::vector<std::string>& children,
    Module* module,
    int param) {

    SubMenuParamCtrl* temporaryThis = new SubMenuParamCtrl(children, module, param, nullptr);
    temporaryThis->text = label;
    menu->addChild(temporaryThis);
    return temporaryThis;
}

inline SubMenuParamCtrl::SubMenuParamCtrl(const std::vector<std::string>& children, Module* module, int param, RenderFunc fun) :
    items(children), module(module), paramNumber(param), func(fun) {
}

inline int SubMenuParamCtrl::getCurrentSetting() {
    return int(std::round(::rack::appGet()->engine->getParam(module, paramNumber)));
}

inline void SubMenuParamCtrl::setParam(int value) {
    ::rack::appGet()->engine->setParam(module, paramNumber, value);
}

inline ::rack::ui::Menu* SubMenuParamCtrl::createChildMenu() {
    ::rack::ui::Menu* menu = new ::rack::ui::Menu();
    int value = 0;
    for (auto item : items) {
        //const char* kluge = item.c_str();

        std::string itemText = this->func ? this->func(value) : item;

        auto isCheckedFn = [this, value]() {
            return this->getCurrentSetting() == value;
        };
        auto onActionFn = [this, value]() {
            this->setParam(value);
        };

        SqMenuItem* mi = new SqMenuItem(itemText.c_str(), isCheckedFn, onActionFn);
        menu->addChild(mi);
        ++value;
    }

    return menu;
}