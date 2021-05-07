#pragma once

class SubMenuParamCtrl : public ::rack::MenuItem {
public:
    static void create(Menu*, const std::string& label, const std::vector<std::string>& children);
    ::rack::ui::Menu* createChildMenu() override;

private:
    SubMenuParamCtrl(const std::vector<std::string>& children);
    std::vector<std::string> items;
};

inline void SubMenuParamCtrl::create(Menu* menu, const std::string& label, const std::vector<std::string>& children) {
    SubMenuParamCtrl* temporaryThis = new SubMenuParamCtrl(children);
    temporaryThis->text = label;
    menu->addChild(temporaryThis);
    
}

inline SubMenuParamCtrl::SubMenuParamCtrl(const std::vector<std::string>& children) : items(children) {
}

/*
   SqMenuItem(const char* label,
                std::function<bool()> isCheckedFn,
               std::function<void()> clickFn) : _isCheckedFn(isCheckedFn),
                                                _onActionFn(clickFn) {
                                                    */
inline ::rack::ui::Menu* SubMenuParamCtrl::createChildMenu() {
    ::rack::ui::Menu* menu = new ::rack::ui::Menu();
    auto label = ::rack::construct<::rack::ui::MenuLabel>(
        &rack::ui::MenuLabel::text,
        "Base octave");
    for (auto item : items) {
        const char* kluge = item.c_str();

        auto isCheckedFn = []() {
            return false;
        };
        auto onActionFn = []() {

        };

        SqMenuItem* mi = new SqMenuItem(kluge, isCheckedFn, onActionFn);
        menu->addChild(mi);
    }
   
    return menu;
}