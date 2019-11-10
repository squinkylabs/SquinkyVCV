
#include "PitchInputWidget.h"
#include "UIPrefs.h"

using Vec = ::rack::math::Vec;
//using Button = ::rack::ui::Button;
using Widget = ::rack::widget::Widget;
using Label = ::rack::ui::Label;

PitchInputWidget::PitchInputWidget(
    const ::rack::math::Vec& pos, 
    const ::rack::math::Vec& siz,
    const std::string& label, 
    bool relative,
    std::vector<InputControl*>& inputControls)
{
    box.pos = pos;
    // TODO: we should set our own height
    box.size = siz;

    // Make the input controls for octave, pitch, and constrain
    // TODO: these are leaking
    // TODO2: these are all fakes
    inputControls.push_back(new InputControlFacade());
    inputControls.push_back(new InputControlFacade());
    inputControls.push_back(new InputControlFacade());


    // add label
    float x= 0;
    float y = 0;
    //auto l = addLabel(Vec(x, y), label.c_str(), UIPrefs::XFORM_TEXT_COLOR);

    Label* labelCtrl = new Label();
    labelCtrl->box.pos = Vec(x, y);
    labelCtrl->text = label.c_str();
    labelCtrl->color = UIPrefs::XFORM_TEXT_COLOR;
    this->addChild(labelCtrl);

    labelCtrl->box.size.x = pos.x - 10;
    labelCtrl->alignment = Label::RIGHT_ALIGNMENT;
  

    // add octave

    // add chromatic semi

    // add scale degrees

    // add checkbox
}

PitchInputWidget::InputControlFacade::~InputControlFacade()
{

}
float PitchInputWidget::InputControlFacade::getValue() const
{
    return 0;
}

void PitchInputWidget::InputControlFacade::setValue(float) 
{

}

void PitchInputWidget::InputControlFacade::enable(bool enabled) 
{

}


void PitchInputWidget::InputControlFacade::setCallback(std::function<void(void)>) 
{

}

  