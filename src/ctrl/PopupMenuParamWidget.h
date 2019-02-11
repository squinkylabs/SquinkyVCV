#pragma once

/**
 * UI Widget that does:
 *  functions as a parameter
 *  pops up a menu of discrete choices.
 *  displays the current choice
 */
class PopupMenuParamWidget : public ChoiceButton
{
public:
      PopupMenuParamWidget(
        int param,
        const Vec& pos,
        float width,
        std::vector<std::string> labels);
private:
      std::vector<std::string> labels;
};

inline PopupMenuParamWidget:: PopupMenuParamWidget(
    int param, 
    const Vec& pos, 
    float width,
    std::vector<std::string> l)
{
    this->text = "testing";
    this->box.pos = pos;
    this->box.size.x = width;
    this->labels = l;
  //  this->box.size.y = 50;          // just for debugging;
}