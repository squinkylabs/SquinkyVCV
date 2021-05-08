#pragma once

#include <string>
#include <vector>

#include "SqMenuItem.h"

class SubMenuParamCtrl : public ::rack::MenuItem {
public:
    static void create(Menu*, const std::string& label, const std::vector<std::string>& children,
                       Module*, int param);
    ::rack::ui::Menu* createChildMenu() override;

private:
    SubMenuParamCtrl(const std::vector<std::string>& children, Module*, int param);
    int getCurrentSetting();
    void setParam(int value);
    const std::vector<std::string> items;
    Module* const module;
    const int paramNumber;
};

inline void SubMenuParamCtrl::create(
    Menu* menu,
    const std::string& label,
    const std::vector<std::string>& children,
    Module* module,
    int param) {
    SubMenuParamCtrl* temporaryThis = new SubMenuParamCtrl(children, module, param);
    temporaryThis->text = label;
    menu->addChild(temporaryThis);
}

inline SubMenuParamCtrl::SubMenuParamCtrl(const std::vector<std::string>& children, Module* module, int param) : items(children), module(module), paramNumber(param) {
}

inline int SubMenuParamCtrl::getCurrentSetting() {
    return int(std::round(::rack::appGet()->engine->getParam(module, paramNumber)));
}

inline void SubMenuParamCtrl::setParam(int value) {
    ::rack::appGet()->engine->setParam(module, paramNumber, value);
}

/*
   SqMenuItem(const char* label,
                std::function<bool()> isCheckedFn,
               std::function<void()> clickFn) : _isCheckedFn(isCheckedFn),
                                                _onActionFn(clickFn) {
                                                    */
inline ::rack::ui::Menu* SubMenuParamCtrl::createChildMenu() {
    ::rack::ui::Menu* menu = new ::rack::ui::Menu();
#if 0
    auto label = ::rack::construct<::rack::ui::MenuLabel>(
        &rack::ui::MenuLabel::text,
        "Base octave");
#endif
    int value = 0;
    for (auto item : items) {
        const char* kluge = item.c_str();

        auto isCheckedFn = [this, value]() {
            return this->getCurrentSetting() == value;
        };
        auto onActionFn = [this, value]() {
            this->setParam(value);
        };

        SqMenuItem* mi = new SqMenuItem(kluge, isCheckedFn, onActionFn);
        menu->addChild(mi);
        ++value;
    }

    return menu;
}